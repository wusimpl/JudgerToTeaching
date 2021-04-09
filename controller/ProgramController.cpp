//
// Created by rory on 2021/3/6.
//

#include <sys/wait.h>
#include "ProgramController.h"
#include "SubProcess.h"

ProcessController::ControllerResult ProcessController::run() {
    ControllerResult controllerResult;
    //检查root权限
    if(getuid() == 0){
        DEBUG_PRINT("需要root权限!");
        controllerResult.runStatus = ControllerResult::PERMISSION_DENIED;
        return controllerResult;
    }

    //
    int subPid = fork(); //vfork()保证子进程先行，此处暂时不用
    struct timeval start{};
    struct timeval end{};
    gettimeofday(&start, nullptr);
    if(subPid == 0){//子进程代码
        DEBUG_PRINT("子进程 running!!");
        SubProcess subProcess(config);
        subProcess.run();
    }else if (subPid > 0){//父进程代码
        DEBUG_PRINT("父进程 running!!");
        //创建监视线程，超时则杀死子进程
        if(config->requiredResourceLimit.realTime == UNLIMITED){
            config->requiredResourceLimit.realTime = 5 minutes; //最多让其运行5分钟
        }
        pthread_t monitorThread;
        ThreadInfo threadInfo = {subPid,config->requiredResourceLimit.realTime};
        int returnValue = pthread_create(&monitorThread, nullptr,ProcessController::timeoutKiller, &threadInfo);
        if( returnValue == RV_OK){
            struct rusage resourceUsage{};
            int status;

            int waitResult = wait4(subPid, &status, WUNTRACED, &resourceUsage);// WUNTRACED:子进程停止、退出时返回(but not traced via trace(2))
            gettimeofday(&end, nullptr);
            config->usedResourceLimit.realTime =  TIME_VALUE(end) - TIME_VALUE(start);
            if (waitResult == RV_ERROR) {
                DEBUG_PRINT("wait4 error");
                controllerResult.runStatus = ControllerResult::WAIT_ERROR;
            }else if(waitResult == subPid){
                if(WIFEXITED(status) || WIFSTOPPED(status)){
                    controllerResult.runStatus = ControllerResult::EXITED_NORMALLY;
                    if(pthread_cancel(monitorThread) != RV_OK){ //关闭监视线程
                        // cancel error
                    }
                    config->usedResourceLimit.cpuTime = R_CPU_TIME(resourceUsage);
                    config->usedResourceLimit.memory = resourceUsage.ru_maxrss KB;
                    config->usedResourceLimit.stack = resourceUsage.ru_idrss KB;
                    config->usedResourceLimit.outputSize = 0; //unused and shouldn't be used

                }
            }
        }else{
            DEBUG_PRINT("thread_create error:" << returnValue);
            KILL_PROCESS(subPid);
            controllerResult.runStatus = ControllerResult::THREAD_CREATE_ERROR;
        }

    }else{
        controllerResult.runStatus = ControllerResult::FORK_ERROR;
    }
    return controllerResult;
}

void *ProcessController::timeoutKiller(void *threadInfo){
    if (pthread_detach(pthread_self()) != 0) { //分离式线程，该子线程结束后自动释放所持资源
        sleep(static_cast<ThreadInfo*>(threadInfo)->timeout / 1000);//休眠：sleep限定的时间，sleep完毕后子进程还在运行则杀死
        int returnValue = KILL_PROCESS(static_cast<ThreadInfo*>(threadInfo)->pid);
        if(returnValue == RV_OK || returnValue == ESRCH){
            DEBUG_PRINT("进程已死亡或已杀死");
            return KILLED_SUCCESS;
        }
    }else{
        return KILLED_ERROR_INFO;
    }
}