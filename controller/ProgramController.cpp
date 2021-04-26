//
// Created by rory on 2021/3/6.
//

#include <sys/wait.h>
#include "ProgramController.h"
#include "SubProcess.h"
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/user.h>

#define DEBUG_SUBPROCESS false
#define SysCallIn 1 // 进入系统调用
#define SysCallOut 0 // 退出系统调用

ControllerResult ProcessController::run() {
    ControllerResult controllerResult;

    //检查root权限 (actually root confirmation has already done in the main function)
    if(getuid() != 0){
        DEBUG_PRINT("需要root权限!");
        controllerResult.runStatus = ControllerResult::PERMISSION_DENIED;
        config->wholeResult.errorCode = WholeResult::SE;
        return controllerResult;
    }


    int subPid = fork();
    struct timeval start{}; // count real time (现实世界中程序从开始到结束的时间)
    struct timeval end{};
    if(subPid == 0){//子进程代码
        DEBUG_PRINT("子进程 running!!");
        // 方便调试子进程，睡眠30s https://ftp.gnu.org/old-gnu/Manuals/gdb/html_node/gdb_25.html
        if(DEBUG_SUBPROCESS){
            sleep(30);
        }

        SubProcess subProcess(config);

        kill(getpid(),SIGSTOP); // 子进程先阻塞自己，等待父进程准备好
        subProcess.runUserProgram();

    }else if (subPid > 0){ // 父进程代码
        DEBUG_PRINT("父进程 running!!");
        DEBUG_PRINT("sub pid:" << subPid);

        if(config->requiredResourceLimit.realTime == UNLIMITED){
            DEBUG_PRINT("Danger! cpu time is unlimited, force to 1 minute.")
            config->requiredResourceLimit.realTime = 1 * minutes; //最多让其运行1分钟
        }
        pthread_t monitorThread; // 创建监视线程，超时则杀死子进程
        pthread_t traceThread; // 创建跟踪进程，拦截exit系统调用，或最后一刻的status信息 (unused，换成在主线程中跟踪)
        pthread_attr_t attr;
        ThreadInfo monitorThreadInfo = {subPid,config->requiredResourceLimit.realTime};
        if(pthread_attr_init(&attr) != RV_OK){
            DEBUG_PRINT("thread attribute initialization failed");
        }
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED); // set thread to unjoinable

        if(pthread_create(&monitorThread, nullptr,ProcessController::timeoutKiller, &monitorThreadInfo) == RV_OK){
            struct rusage resourceUsage{};
            int wstatus;
            int waitResult;
            string subProcessExitedStatus; // 子线程最后一刻(exit调用前)的/proc/[pid]/status状态信息
            struct user_regs_struct regs{};
            bool sysCallStatus = SysCallIn;

            while(true){
                ptrace(PTRACE_SYSCALL, subPid, nullptr, nullptr); // 跟踪子进程的系统调用

//                kill(subPid,SIGCONT); // send signal to subprocess to make it continue to run.
                waitResult = wait4(subPid, &wstatus, WUNTRACED, &resourceUsage);

                if (waitResult == RV_ERROR) {
                    switch (errno) {
                        case ECHILD: // 主线程可能等不到子进程返回就因为超时被监视线程杀死了,或者子进程先行，已经在父进程继续执行前就已经运行完毕
                            DEBUG_PRINT("子进程不存在!");
                            break;
                        case EINVAL:
                            DEBUG_PRINT("wait4 参数错误!")
                            break;
                    }
                    break;
                }else if(waitResult == subPid) {
                    if (WIFEXITED(wstatus)) { // the child terminated normally

                        // 存储运行结果与返回值
                        int returnValue = WEXITSTATUS(wstatus); // the return value of user programme
                        controllerResult.runStatus = ControllerResult::EXITED_NORMALLY;
                        controllerResult.returnValue = returnValue;

                        // 获取程序占用资源
                        gettimeofday(&end, nullptr);
                        config->usedResourceLimit.realTime =  (TIME_VALUE(end) - TIME_VALUE(start))/1000;
                        config->usedResourceLimit.cpuTime = R_CPU_TIME(resourceUsage)/1000;
                        //十分不精确的测量，已弃用，使用/proc/[pid]/status实现
                        config->usedResourceLimit.memory = resourceUsage.ru_maxrss;
                        config->usedResourceLimit.stack = resourceUsage.ru_isrss;
                        config->usedResourceLimit.outputSize = 0;

                        DEBUG_PRINT(config->usedResourceLimit.toString());
                        DEBUG_PRINT("procStatus:" << subProcessExitedStatus);
                        break;
                    } else if (WIFSIGNALED(wstatus)) { // terminated by a signal
                        DEBUG_PRINT("terminated signal:" << WTERMSIG(wstatus));
                        switch (WTERMSIG(wstatus)) {
                            case SIGSEGV: // 栈、内存超限会被发送此信号
                                config->wholeResult.errorCode = WholeResult::MLE;
                                break;
                            case SIGXCPU: // cpu超时会被发送此信号
                                config->wholeResult.errorCode = WholeResult::CLE;
                                break;
                            case SIGXFSZ: // 写入文件的数据超限会发送次信号
                                config->wholeResult.errorCode = WholeResult::OLE;
                                break;
                            default:
                                DEBUG_PRINT("捕获到其他信号！");
                                break;
                        }

                    } else if (WIFSTOPPED(wstatus)) { // stopped by a signal
                        DEBUG_PRINT("stopped signal:" << WSTOPSIG(wstatus));

//                        ptrace(PTRACE_GETREGS, subPid, 0, &regs); // 获取系统调用参数(主要是系统调用号)

                        switch (WSTOPSIG(wstatus)) {
                            case SIGCONT:
                                break;
                            case SIGSTOP: // 子进程阻塞了自己，运行到这里子进程就可以开始运行用户代码了
                                gettimeofday(&start, nullptr);
                                kill(subPid,SIGCONT);
                                break;
                            case SIGTRAP:
                                if(regs.orig_rax == SYS_exit_group) {
                                    if (sysCallStatus == SysCallIn) { // 进入系统调用
                                        sysCallStatus = SysCallOut;
                                        string command = string("cat /proc/") + std::to_string(subPid) + string("/status");
                                        execShellCommand(command, subProcessExitedStatus);
                                        DEBUG_PRINT("traced SYS_exit successfully")
                                    } else { // 退出系统调用
                                        sysCallStatus = SysCallIn;
                                    }
                                }
                                break;
                        }


                    } else if (WIFCONTINUED(wstatus)) {
                        DEBUG_PRINT("resumed by delivery of SIGCONT.");
                    }
                }
            }
        }else{
            DEBUG_PRINT("thread_create error");
            KILL_PROCESS(subPid);
            controllerResult.runStatus = ControllerResult::THREAD_CREATE_ERROR;
        }

        if(pthread_cancel(monitorThread) != RV_OK){ //关闭监视线程
            // cancel error
            DEBUG_PRINT("监视线程未正常退出!");
        }

    }else{
        controllerResult.runStatus = ControllerResult::FORK_ERROR;
    }
    return controllerResult;
}


void *ProcessController::timeoutKiller(void *threadInfo){
    auto* threadInfo1 = static_cast<ThreadInfo*>(threadInfo);
    DEBUG_PRINT("monitor thread started!");
    DEBUG_PRINT("sleep time:" << int(threadInfo1->timeout));
    sleep(int(threadInfo1->timeout));//休眠：sleep限定的时间，sleep完毕后子进程还在运行则杀死
    int returnValue = KILL_PROCESS(threadInfo1->pid);
    if(returnValue == RV_OK || returnValue == ESRCH){
        DEBUG_PRINT("进程已死亡或已杀死");
        return KILLED_SUCCESS;
    }
    return KILLED_ERROR_INFO;
}