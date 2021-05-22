//
// Created by WilliamsAndy on 2021/4/9.
//

#include "SubProcess.h"
#include <sys/ptrace.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include "SeccompRules.h"
#include <signal.h>
#define AddSysCallRule(sysCallNumber) \
    seccomp_rule_add(ctx,config->fileType==WHITE_LIST_MODE?SCMP_ACT_ALLOW:SCMP_ACT_KILL,sysCallNumber,0)

SubProcess::SubProcess(JudgeConfig *cfg):config(cfg) {
    //变量初始化
    for (int i = 0; i < MAX_TEST_FILE_NUMBER; ++i) {
        openedReadFiles[i] = openedWriteFiles[i] = nullptr;
    }
    ctx = nullptr;
    //资源限制
    setResourceLimit();
    //重定向输入输出
    redirectIO();
    //信号捕捉
//    signal(SIGXCPU,xcpuSignalHandler);
    //配置系统调用过滤规则
//    restrainSystemCall();
    //trace me
    ptrace(PTRACE_TRACEME);
}

void SubProcess::setResourceLimit() {
    DEBUG_PRINT("设置资源限制中...");
    //超出限制会被发送SIGSEGV信号杀死进程
    if(config->requiredResourceLimit.memory != UNLIMITED){ // in KB
        // in bytes
        rlimit as = {static_cast<rlim_t>(config->requiredResourceLimit.memory*KB), static_cast<rlim_t>(config->requiredResourceLimit.memory*KB*2)}; //max memory size
        DEBUG_PRINT("限制内存大小"<<config->requiredResourceLimit.memory*KB<<"bytes");
        if(SetRLimit_X(AS,as) != 0){
            DEBUG_PRINT("资源限制错误!");
            return;
        }
    }

    //超出软限制会被发送SIGXCPU信号，如果没有捕获该信号，则被杀死
    if(config->requiredResourceLimit.cpuTime != UNLIMITED){ // cpu time:in seconds
        // in seconds
        rlimit cpu = {unsigned(config->requiredResourceLimit.cpuTime/1000), unsigned(config->requiredResourceLimit.cpuTime/200)};
        DEBUG_PRINT(cpu.rlim_cur<<" "<<cpu.rlim_max);
        if(SetRLimit_X(CPU,cpu) != 0){
            DEBUG_PRINT("资源限制错误!");
            return;
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
}

bool SubProcess::restrainSystemCall() {
//    int syscalls_whitelist[] = {SCMP_SYS(read), SCMP_SYS(fstat),
//                                SCMP_SYS(mmap), SCMP_SYS(mprotect),
//                                SCMP_SYS(munmap), SCMP_SYS(uname),
//                                SCMP_SYS(arch_prctl), SCMP_SYS(brk),
//                                SCMP_SYS(access), SCMP_SYS(exit_group),
//                                SCMP_SYS(close), SCMP_SYS(readlink),
//                                SCMP_SYS(sysinfo), SCMP_SYS(write),
//                                SCMP_SYS(writev), SCMP_SYS(lseek),
//                                SCMP_SYS(clock_gettime), SCMP_SYS(ptrace),
//                                SCMP_SYS(execve)};
    int syscalls_whitelist[] = {
                                SCMP_SYS(brk),
                                SCMP_SYS(exit_group),
                                SCMP_SYS(ptrace),
                                SCMP_SYS(execve)};

    int syscalls_whitelist_length = sizeof(syscalls_whitelist) / sizeof(int);

//    DEBUG_PRINT("限制系统调用中...");
    ctx = seccomp_init(SCMP_ACT_ERRNO(1000)); // 黑白名单模式设置;

    // 必须允许的系统调用
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0); //scanf is allowed
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0); //printf is allowed
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 1,
                     SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t) config->exePath.c_str())); // executing user program specified by exePath is allowed


    if(ctx == nullptr){
        DEBUG_PRINT("seccomp初始化失败");
        return false;
    }

    if(seccomp_load(ctx) != RV_OK){
        DEBUG_PRINT("seccomp load error!");
        return false;
    }
    seccomp_release(ctx);
    return true;
}


void SubProcess::redirectIO() {
    if(!config->testInPath.empty()){
        FILE* inputFile = fopen(config->testInPath.c_str(),"r");
        if(inputFile== nullptr){
            DEBUG_PRINT("文件打开错误!");
        }
        if((dup2(fileno(inputFile),fileno(stdin)) == -1)){
            DEBUG_PRINT("重定向错误！");
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

}

void SubProcess::xcpuSignalHandler(int sig) {
    DEBUG_PRINT("SIGUSER1");
    kill(getpid(),SIGUSR1); //通知父进程
}
