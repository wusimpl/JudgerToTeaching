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

ProcessController::ControllerResult ProcessController::run() {
    ControllerResult controllerResult;

    //检查root权限 (actually root confirmation has already done in the main function)
    if(getuid() != 0){
        DEBUG_PRINT("需要root权限!");
        controllerResult.runStatus = ControllerResult::PERMISSION_DENIED;
        return controllerResult;
    }


    int subPid = vfork(); //vfork()保证子进程先行，此处暂时不用
    struct timeval start{}; // count real time (现实世界中程序从开始到结束的时间)
    struct timeval end{};
    gettimeofday(&start, nullptr);
    if(subPid == 0){//子进程代码
        DEBUG_PRINT("子进程 running!!");
        if(DEBUG_SUBPROCESS){ // 方便调试子进程，睡眠30s https://ftp.gnu.org/old-gnu/Manuals/gdb/html_node/gdb_25.html
            sleep(30);
        }
        SubProcess subProcess(config);
        subProcess.runUserProgram();

    }else if (subPid > 0){ // 父进程代码
        DEBUG_PRINT("父进程 running!!");
        DEBUG_PRINT("sub pid:" << subPid);

        if(config->requiredResourceLimit.realTime == UNLIMITED){
            config->requiredResourceLimit.realTime = 1 * minutes; //最多让其运行1分钟
        }
        pthread_t monitorThread; // 创建监视线程，超时则杀死子进程
//        pthread_t traceThread; // 创建跟踪进程，拦截exit系统调用，或最后一刻的status信息 (unused，换成在主线程中跟踪)
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
//            string processStatus;
            while(true){
                ptrace(PTRACE_SYSCALL, subPid, nullptr, nullptr); // 跟踪系统调用

                waitResult = wait4(subPid, &wstatus, WUNTRACED, &resourceUsage);

                if (waitResult == RV_ERROR) {
                    DEBUG_PRINT("wait4 error");
                    controllerResult.runStatus = ControllerResult::WAIT_ERROR;
                }else if(waitResult == subPid) {
                    if (WIFEXITED(wstatus)) { // the child terminated normally
                        int returnValue = WEXITSTATUS(wstatus); // the return value of user programme
                        gettimeofday(&end, nullptr);
                        config->usedResourceLimit.realTime =  TIME_VALUE(end) - TIME_VALUE(start);

                        controllerResult.runStatus = ControllerResult::EXITED_NORMALLY;
                        config->usedResourceLimit.cpuTime = R_CPU_TIME(resourceUsage);

                        //十分不精确的测量，已弃用，使用/proc/[pid]/status实现
                        config->usedResourceLimit.memory = resourceUsage.ru_maxrss;
                        config->usedResourceLimit.stack = resourceUsage.ru_isrss;
                        config->usedResourceLimit.outputSize = 0;

                        DEBUG_PRINT(config->usedResourceLimit.toString());
                        DEBUG_PRINT("procStatus:" << subProcessExitedStatus);
                        break;
                    } else if (WIFSIGNALED(wstatus)) { // terminated by a signal
                        DEBUG_PRINT("terminated signal:" << WTERMSIG(wstatus));
                    } else if (WIFSTOPPED(wstatus)) { // stopped by a signal
                        DEBUG_PRINT("stopped signal:" << WSTOPSIG(wstatus));
                        ptrace(PTRACE_GETREGS, subPid, 0, &regs); // 获取系统调用参数(主要是系统调用号)
//                        DEBUG_PRINT("syscall:"<< regs.orig_rax);
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
        }

    }else{
        controllerResult.runStatus = ControllerResult::FORK_ERROR;
    }
    return controllerResult;
}

void *ProcessController::timeoutKiller(void *threadInfo){
    DEBUG_PRINT("monitor thread started!");
    sleep(int(static_cast<ThreadInfo*>(threadInfo)->timeout) / 1000);//休眠：sleep限定的时间，sleep完毕后子进程还在运行则杀死
    int returnValue = KILL_PROCESS(static_cast<ThreadInfo*>(threadInfo)->pid);
    if(returnValue == RV_OK || returnValue == ESRCH){
        DEBUG_PRINT("进程已死亡或已杀死");
        return KILLED_SUCCESS;
    }
    return KILLED_ERROR_INFO;
}