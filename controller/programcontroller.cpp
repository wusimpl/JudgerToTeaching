//
// Created by root on 2021/4/23.
//

#include "programcontroller.h"

#include <sys/wait.h>
#include "ProgramController.h"
#include "subprocess.h"

#define DEBUG_SUBPROCESS false

ControllerResult control(JudgeConfig* config) {
    ControllerResult controllerResult;
    //检查root权限
    if(getuid() != 0){
        DEBUG_PRINT("需要root权限!");
        controllerResult.runStatus = PERMISSION_DENIED;
        return controllerResult;
    }


    int subPid = fork(); // vfork()保证子进程先行，此处暂时不用
    struct timeval start{};
    struct timeval end{};
    gettimeofday(&start, nullptr);
    if(subPid == 0){ // 子进程代码
        DEBUG_PRINT("子进程 running!!");
        if(DEBUG_SUBPROCESS){ // 方便调试子进程，睡眠20s https://ftp.gnu.org/old-gnu/Manuals/gdb/html_node/gdb_25.html
            sleep(30);
        }
        subProcessInit(config);
//        runUserProgram(config);
        execve("/root/test/a.out", nullptr, nullptr);

    }else if (subPid > 0){ // 父进程代码
        DEBUG_PRINT("父进程 running!!");
        //创建监视线程，超时则杀死子进程
        if(config->requiredResourceLimit.realTime == UNLIMITED){
            config->requiredResourceLimit.realTime = 5 minutes; //最多让其运行5分钟
        }
        pthread_t monitorThread;
        pthread_attr_t attr;
        ThreadInfo threadInfo = {subPid,config->requiredResourceLimit.realTime};
        if(pthread_attr_init(&attr) != RV_OK){
            DEBUG_PRINT("thread attribute initialization failed");
        }
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED); // set thread to unjoinable

        if( pthread_create(&monitorThread, nullptr,timeoutKiller, &threadInfo) == RV_OK ){
            struct rusage resourceUsage{};
            int wstatus;
            int waitResult = wait4(subPid, &wstatus, WUNTRACED, &resourceUsage);// WUNTRACED:子进程停止、退出时返回(but not traced via trace(2))
            gettimeofday(&end, nullptr);
            config->usedResourceLimit.realTime =  TIME_VALUE(end) - TIME_VALUE(start);

            if(pthread_cancel(monitorThread) != RV_OK){ //关闭监视线程
                // cancel error
            }

            if (waitResult == RV_ERROR) {
                DEBUG_PRINT("wait4 error");
                controllerResult.runStatus = WAIT_ERROR;
            }else if(waitResult == subPid){
                if(WIFEXITED(wstatus)){ // the child terminated normally
                    int returnValue = WEXITSTATUS(wstatus); // the return value of user programme

                    controllerResult.runStatus = EXITED_NORMALLY;
                    config->usedResourceLimit.cpuTime = R_CPU_TIME(resourceUsage);
                    config->usedResourceLimit.memory = resourceUsage.ru_maxrss KB;
                    config->usedResourceLimit.stack = resourceUsage.ru_idrss KB;
                    config->usedResourceLimit.outputSize = 0; //unused and shouldn't be used

                }else if(WIFSIGNALED(wstatus)){ // terminated by a signal
                    DEBUG_PRINT("terminated signal:" << WTERMSIG(wstatus));
                }else if(WIFSTOPPED(wstatus)){ // stopped by a signal
                    DEBUG_PRINT("stopped signal:" << WSTOPSIG(wstatus));
                }else if(WIFCONTINUED(wstatus)){
                    DEBUG_PRINT("resumed by delivery of SIGCONT.");
                }
            }
        }else{
            DEBUG_PRINT("thread_create error");
            KILL_PROCESS(subPid);
            controllerResult.runStatus = THREAD_CREATE_ERROR;
        }

    }else{
        controllerResult.runStatus = FORK_ERROR;
    }
    return controllerResult;
}

void* timeoutKiller(void *threadInfo){
    DEBUG_PRINT("monitor thread started!");
    sleep(static_cast<ThreadInfo*>(threadInfo)->timeout / 1000);//休眠：sleep限定的时间，sleep完毕后子进程还在运行则杀死
    int returnValue = KILL_PROCESS(static_cast<ThreadInfo*>(threadInfo)->pid);
    if(returnValue == RV_OK || returnValue == ESRCH){
        DEBUG_PRINT("进程已死亡或已杀死");
        return KILLED_SUCCESS;
    }
    return KILLED_ERROR_INFO;
}