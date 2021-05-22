//
// Created by WilliamsAndy on 2021/4/9.
//

#include "SubProcess.h"
#include <sys/ptrace.h>
#include "SeccompRules.h"
#include <signal.h>
#include <fcntl.h>
#define AddSysCallRule(sysCallNumber) \
    seccomp_rule_add(ctx,config->fileType==WHITE_LIST_MODE?SCMP_ACT_ALLOW:SCMP_ACT_KILL,sysCallNumber,0)

SubProcess::SubProcess(JudgeConfig *cfg,int processId):config(cfg) {
    //变量初始化
    for (int i = 0; i < MAX_TEST_FILE_NUMBER; ++i) {
        openedReadFiles[i] = openedWriteFiles[i] = nullptr;
    }
    ctx = nullptr;
    this->pid = processId;
    //资源限制
    initSuccess = setResourceLimit();
    //traceable enabled
    ptrace(PTRACE_TRACEME);
    //重定向输入输出
    initSuccess = redirectIO();
    //配置系统调用过滤规则
    initSuccess = restrainSystemCall();
}

int SubProcess::setResourceLimit() {
    DEBUG_PRINT("设置资源限制中...");
    //超出限制会被发送SIGSEGV信号杀死进程
    if(config->requiredResourceLimit.memory != UNLIMITED){ // in KB
        // in bytes
        rlimit as = {static_cast<rlim_t>(config->requiredResourceLimit.memory*KB), static_cast<rlim_t>(config->requiredResourceLimit.memory*KB*2)}; //max memory size
        DEBUG_PRINT("限制内存大小"<<config->requiredResourceLimit.memory*KB<<"bytes");
        if(SetRLimit_X(AS,as) != 0){
            DEBUG_PRINT("资源限制错误!");
            return false;
        }
    }

    //超出软限制会被发送SIGXCPU信号，如果没有捕获该信号，则被杀死
    if(config->requiredResourceLimit.cpuTime != UNLIMITED){ // cpu time:in seconds
        // in seconds
        rlimit cpu = {unsigned(config->requiredResourceLimit.cpuTime/1000), unsigned(config->requiredResourceLimit.cpuTime/200)};
        DEBUG_PRINT(cpu.rlim_cur<<" "<<cpu.rlim_max);
        if(SetRLimit_X(CPU,cpu) != 0){
            DEBUG_PRINT("资源限制错误!");
            return false;
        }
    }

    //SIGSEGV would be sent
//    if(config->requiredResourceLimit.stack != UNLIMITED){
//        rlimit stack = {static_cast<rlim_t>(config->requiredResourceLimit.stack*KB), static_cast<rlim_t>(config->requiredResourceLimit.stack*KB*2)};
//        if(SetRLimit_X(STACK,stack) != 0){
//            DEBUG_PRINT("资源限制错误!");
//            return;
//        }
//    }
//    // 超出限制被发送SIGXFSZ信号
//    if(config->requiredResourceLimit.outputSize != UNLIMITED){
//        rlimit fsize = {static_cast<rlim_t>(config->requiredResourceLimit.outputSize*KB), static_cast<rlim_t>(config->requiredResourceLimit.outputSize*KB*2)};
//        if(SetRLimit_X(FSIZE,fsize) != 0){
//            DEBUG_PRINT("资源限制错误!");
//            return;
//        }
//    }
    return true;
}

int SubProcess::runUserProgram() {
    //cpu控制权移交给被测程序
//    DEBUG_PRINT("开始运行用户程序..."); //不要再在函数进行任何输出，否则会写入用户程序的输出文件，导致无法进行正确的答案检查
    if(execve(config->exePath.c_str(), config->programArgs, nullptr) != RV_OK){
//    if(execl( "/bin/sh", "sh", "-c", config->exePath.c_str(), nullptr) != RV_OK){
        DEBUG_PRINT("failed to execute user program!");
        DEBUG_PRINT("errno:" << errno);
        return errno;
    }
}

// may never be called
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
//    if(ctx != nullptr){
//        seccomp_release(ctx);
//    }
}

int SubProcess::restrainSystemCall() {
    int syscallWhiteList[] = {
            SCMP_SYS(write),SCMP_SYS(read), // allow printf and write
            SCMP_SYS(mmap), SCMP_SYS(munmap),SCMP_SYS(brk), // allow memory allocation and free
            SCMP_SYS(exit_group),SCMP_SYS(rt_sigreturn), // allow exit
            SCMP_SYS(clock_gettime) // allow get time
    };

    ctx = seccomp_init(SCMP_ACT_KILL); // init

    if(ctx == nullptr){
        DEBUG_PRINT("seccomp初始化失败");
        return false;
    }

    //必须允许的系统调用
    int rv;
    for (int i = 0; i < sizeof(syscallWhiteList); ++i) {
        rv = seccomp_rule_add(ctx,SCMP_ACT_ALLOW,syscallWhiteList[i],0);
        if(rv!=0){
            return false;
        }
    }
    //allow subprocess to stop itself.
    seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(kill),2,
                     SCMP_A0( SCMP_CMP_EQ, static_cast<scmp_datum_t>(pid) ),
                     SCMP_A1( SCMP_CMP_EQ, (scmp_datum_t)SIGSTOP ) );
    //允许执行用户程序
    if (seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(execve), 1, SCMP_A0(SCMP_CMP_NE, (scmp_datum_t)(config->exePath.c_str()))) != 0) {
        return false;
    }

//    if (seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(open), 1, SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_WRONLY, O_WRONLY)) != 0) {
//        return false;
//    }
//    if (seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(open), 1, SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_RDWR, O_RDWR)) != 0) {
//        return false;
//    }

    if(seccomp_load(ctx) != RV_OK){
        DEBUG_PRINT("seccomp load error!");
        return false;
    }
    seccomp_release(ctx);
    return true;
}


int SubProcess::redirectIO() {
    if(!config->testInPath.empty()){
        FILE* inputFile = fopen(config->testInPath.c_str(),"r");
        if(inputFile== nullptr){
            DEBUG_PRINT("文件打开错误!");
            return false;
        }
        if((dup2(fileno(inputFile),fileno(stdin)) == -1)){
            DEBUG_PRINT("重定向错误！");
            return false;
        }
    }

    if(!config->outputFilePath.empty()){
        FILE* outputFile = fopen(config->outputFilePath.c_str(),"w");
        if(outputFile== nullptr){
            DEBUG_PRINT("文件打开错误!");
        }
        if(dup2(fileno(outputFile),fileno(stdout)) == -1){
            DEBUG_PRINT("重定向错误！");
        }
    }
    return true;
}