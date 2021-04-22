//
// Created by WilliamsAndy on 2021/4/9.
//

#include "SubProcess.h"

#define AddSysCallRule(sysCallNumber) seccomp_rule_add(ctx,config->fileType==BLACK_LIST_MODE?SCMP_ACT_ALLOW:SCMP_ACT_KILL,sysCallNumber,0)

SubProcess::SubProcess(JudgeConfig *cfg):config(cfg) {
    //变量初始化
    for (int i = 0; i < MAX_TEST_FILE_NUMBER; ++i) {
        openedReadFiles[i] = openedWriteFiles[i] = nullptr;
    }
    ctx = nullptr;
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
    //配置系统调用过滤规则
    restrainSystemCall();
}

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
    //控制权移交给被测程序
    execv(config->exePath.c_str(), config->programArgs);
//    exit(0);
}

SubProcess::~SubProcess() {
    DEBUG_PRINT("子进程析构中");
    // 释放读写资源
    for (auto & openedReadFile : openedReadFiles) {
        if(openedReadFile != nullptr){
            fclose(openedReadFile);
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
    // 释放系统调用过滤器
    if(ctx != nullptr){
        seccomp_release(ctx);
    }
}

bool SubProcess::restrainSystemCall() {
    ctx = seccomp_init(config->sysCallFilterMode == WHITE_LIST_MODE ? SCMP_ACT_KILL : SCMP_ACT_ALLOW); // 黑白名单模式设置;
    if(ctx == nullptr){
        DEBUG_PRINT("seccomp初始化失败");
        return false;
    }

//    seccomp_rule_add(ctx,SCMP_ACT_ALLOW,SCMP_SYS(execve),0);

    int rv; // return value
    for (int sysCallNumber : config->sysCallList) {
        if(sysCallNumber < 0){
            return true;
        }else {
//            rv = AddSysCallRule(sysCallNumber);
            rv =  seccomp_rule_add(ctx,config->sysCallFilterMode == BLACK_LIST_MODE ? SCMP_ACT_KILL : SCMP_ACT_ALLOW,sysCallNumber,0); // syscallNumber必须由SCMP_SYS()宏来获得
            if(rv < RV_OK){
                DEBUG_PRINT("系统调用设置出错！请检查系统调用表");
                DEBUG_PRINT("rv:" << rv);
                DEBUG_PRINT("errno:" << errno);
                return false;
            }
        }
    }
    if(seccomp_load(ctx) != RV_OK){
        DEBUG_PRINT("seccomp load error!");
        return false;
    }
    return true;

}
