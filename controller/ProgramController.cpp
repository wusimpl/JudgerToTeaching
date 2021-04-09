//
// Created by rory on 2021/3/6.
//

#include "ProgramController.h"


#define minutes *60*1000 // 1 min = 60 000 ms
#define TIME_VALUE(timeval) (timeval.tv_sec*1000 + timeval.tv_usec/1000) // ms
#define R_CPU_TIME(rusage) TIME_VALUE(rusage.ru_utime) // ms

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


SubProcess::SubProcess(JudgeConfig *cfg):config(cfg) {
    //变量初始化
    for (int i = 0; i < MAX_TEST_FILE_NUMBER; ++i) {
        openedReadFiles[i] = openedWriteFiles[i] = nullptr;
    }

    //资源限制
    setResourceLimit();
    //重定向输入输出：打开的文件会在析构函数被调用时被关闭
    Dir testInFiles = getFilesOfDirWithFullPath(cfg->testInPath);
    if(testInFiles.size > 0){
        FILE* inputFile = fopen(testInFiles.files[0].c_str(),"r");
        openedReadFiles[0] = inputFile;
        if((dup2(fileno(inputFile),fileno(stdin)) == -1)){
            DEBUG_PRINT("重定向错误！");
        }
    }
    Dir testOutFiles = getFilesOfDir(cfg->testOutPath); // 依据测试输出文件的数量来确定被测程序的输出文件
    if(testOutFiles.size > 0){
        string file = cfg->outputFilePath.append(testOutFiles.files[0]);
        FILE* outputFile = fopen(file.c_str(),"w");
        openedWriteFiles[0] = outputFile;
        if(dup2(fileno(outputFile),fileno(stdout)) == -1){
            //error
            DEBUG_PRINT("重定向错误！");
        }
    }
}

/**
 * 设置资源限制的函数
 */
void SubProcess::setResourceLimit() {
    if(config->requiredResourceLimit.memory != UNLIMITED){
        rlimit as = {config->requiredResourceLimit.memory, config->requiredResourceLimit.memory}; //max memory size
        if(SetRLimit_X(AS,as) != 0){
            DEBUG_PRINT("资源限制错误!");
            return;
        }
    }
    if(config->requiredResourceLimit.cpuTime != UNLIMITED){
        rlimit cpu = {config->requiredResourceLimit.cpuTime, config->requiredResourceLimit.cpuTime};
        if(SetRLimit_X(CPU,cpu) != 0){
            DEBUG_PRINT("资源限制错误!");
            return;
        }
    }
    if(config->requiredResourceLimit.stack != UNLIMITED){
        rlimit stack = {config->requiredResourceLimit.stack, config->requiredResourceLimit.stack};
        if(SetRLimit_X(STACK,stack) != 0){
            DEBUG_PRINT("资源限制错误!");
            return;
        }
    }
    if(config->requiredResourceLimit.outputSize != UNLIMITED){
        rlimit fsize = {config->requiredResourceLimit.outputSize, config->requiredResourceLimit.outputSize};
        if(SetRLimit_X(FSIZE,fsize) != 0){
            DEBUG_PRINT("资源限制错误!");
            return;
        }
    }
}

void SubProcess::run() {

    //限制系统调用

    //控制权移交给被测程序
    execv(config->exePath.c_str(), config->programArgs);
//    exit(0);
}

SubProcess::~SubProcess() {
    for (auto & openedReadFile : openedReadFiles) {
        if(openedReadFile != nullptr){
            fclose(openedReadFile);
            DEBUG_PRINT("子进程析构中");
        }else{
            break;
        }
    }
    for (auto & openedWriteFile : openedWriteFiles) {
        if(openedWriteFile != nullptr){
            fclose(openedWriteFile);
        }else{
            break;
        }
    }
}
