//
// Created by rory on 2021/3/6.
//

#include "ProgramController.h"
#include <cstdlib>

ProcessController::ControllerResult ProcessController::run() {
    ControllerResult controllerResult;
    //检查root权限
    if(getuid() == 0){
        DEBUG_PRINT("需要root权限!");
        controllerResult.runStatus = ControllerResult::PERMISSION_DENIED;
        return controllerResult;
    }

    //
    int subPid = vfork(); //保证子进程先行
    if(subPid == 0){//子进程代码
        DEBUG_PRINT("子进程 running!!");
        SubProcess subProcess(config);
        subProcess.run();
    }else if (subPid > 0){//父进程代码
        DEBUG_PRINT("父进程 running!!");
        struct rusage resourceUsage{};
        int status;

        if (wait4(subPid, &status, WSTOPPED, &resourceUsage) == -1) {// -1 means error occurred
            DEBUG_PRINT("子进程停止");
            cout<<"子进程发生错误"<<endl;
        }else{
            controllerResult.runStatus = ControllerResult::OK;
            cout<<"父进程完毕"<<endl;
            return controllerResult;
        }
    }else{
        controllerResult.runStatus = ControllerResult::FORK_ERROR;
        return controllerResult;
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
            //error
        }
    }
    Dir testOutFiles = getFilesOfDir(cfg->testOutPath); // 依据测试输出文件的数量来确定被测程序的输出文件
    if(testOutFiles.size > 0){
        string file = cfg->outputFilePath.append(testOutFiles.files[0]);
        FILE* outputFile = fopen(file.c_str(),"w");
        openedWriteFiles[0] = outputFile;
        if(dup2(fileno(outputFile),fileno(stdout)) == -1){
            //error
        }
    }
}

/**
 * 设置资源限制的函数
 */
void SubProcess::setResourceLimit() {
    if(config->requiredResourceLimit.limitedMemory != UNLIMITED){
        rlimit as = {config->requiredResourceLimit.limitedMemory,config->requiredResourceLimit.limitedMemory}; //max memory size
        if(SetRLimit_X(AS,as) != 0){
            DEBUG_PRINT("资源限制错误!");
            return;
        }
    }
    if(config->requiredResourceLimit.limitedCPUTime != UNLIMITED){
        rlimit cpu = {config->requiredResourceLimit.limitedCPUTime,config->requiredResourceLimit.limitedCPUTime};
        if(SetRLimit_X(CPU,cpu) != 0){
            DEBUG_PRINT("资源限制错误!");
            return;
        }
    }
    if(config->requiredResourceLimit.limitedStack != UNLIMITED){
        rlimit stack = {config->requiredResourceLimit.limitedStack,config->requiredResourceLimit.limitedStack};
        if(SetRLimit_X(STACK,stack) != 0){
            DEBUG_PRINT("资源限制错误!");
            return;
        }
    }
    if(config->requiredResourceLimit.limitedOutputSize != UNLIMITED){
        rlimit fsize = {config->requiredResourceLimit.limitedOutputSize,config->requiredResourceLimit.limitedOutputSize};
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
            cout<<"子进程析构中"<<endl;
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
