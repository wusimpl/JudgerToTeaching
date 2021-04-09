//
// Created by WilliamsAndy on 2021/4/9.
//

#include "SubProcess.h"


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