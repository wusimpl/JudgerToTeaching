//
// Created by WilliamsAndy on 2021/4/9.
//

#include "SubProcess.h"
#include <sys/ptrace.h>
#include <csignal>
#include <iostream>
#include <ostream>
//#include "seccomp-bpf.h"
#include <seccomp.h>
#include "kafel.h"
#include <sys/prctl.h>

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
    initSuccess = restrainSystemCall1();
}

int SubProcess::setResourceLimit() {
    DEBUG_PRINT("设置资源限制中...");
    //超出限制会被发送SIGSEGV信号杀死进程
    if(config->requiredResourceLimit.memory != UNLIMITED){ // in KB
        // in bytes
        rlimit as = {static_cast<rlim_t>(config->requiredResourceLimit.memory*KB), static_cast<rlim_t>(config->requiredResourceLimit.memory*KB*2)}; //max memory size
//        DEBUG_PRINT("限制内存大小"<<config->requiredResourceLimit.memory*KB<<"bytes");
        if(SetRLimit_X(AS,as) != 0){
            DEBUG_PRINT("资源限制错误!");
            return false;
        }
    }

    //超出软限制会被发送SIGXCPU信号，如果没有捕获该信号，则被杀死
    if(config->requiredResourceLimit.cpuTime != UNLIMITED){ // cpu time:in seconds
        // in seconds
        rlimit cpu = {unsigned(config->requiredResourceLimit.cpuTime/1000), unsigned(config->requiredResourceLimit.cpuTime/200)};
//        DEBUG_PRINT(cpu.rlim_cur<<" "<<cpu.rlim_max);
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

    if((ctx = seccomp_init(SCMP_ACT_TRAP)) == nullptr){
        DEBUG_PRINT("seccomp init failed!");
        return false;
    }
    int sysCallWhiteList[] = {
//                SCMP_SYS(execve),
            SCMP_SYS(brk),
            SCMP_SYS(arch_prctl),SCMP_SYS(access),
            SCMP_SYS(openat),SCMP_SYS(close),SCMP_SYS(mmap),SCMP_SYS(munmap),
            SCMP_SYS(mprotect),
//                SCMP_SYS(write),SCMP_SYS(read),
            SCMP_SYS(exit_group),SCMP_SYS(fstat), SCMP_SYS(pread64)
    };
    for (int i = 0; i < sizeof sysCallWhiteList; ++i) {
        if(seccomp_rule_add(ctx,SCMP_ACT_ALLOW,sysCallWhiteList[i],0) != RV_OK){
            DEBUG_PRINT("seccomp init failed!");
            return false;
        }
    }
    //允许执行用户程序
    if(seccomp_rule_add(ctx,
                     SCMP_ACT_ALLOW,
                     SCMP_SYS(execve),
                     1,
                     SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t) config->exePath.c_str())) != RV_OK){
        DEBUG_PRINT("seccomp init failed!");
        return false;
    }
    //allow subprocess to stop itself.
    if(seccomp_rule_add(ctx,
                     SCMP_ACT_ALLOW,
                     SCMP_SYS(kill),
                     2,
                     SCMP_A0( SCMP_CMP_EQ, static_cast<scmp_datum_t>(pid) ),
                     SCMP_A1( SCMP_CMP_EQ, (scmp_datum_t)SIGSTOP ) ) != RV_OK){
        DEBUG_PRINT("seccomp init failed!");
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

int SubProcess::restrainSystemCall1() {
    string seccomp_policy;
    char buf[1024];
    const char* fileName = config->seccompFilterFilePath.c_str();
    fstream file(fileName);
    if(!file.is_open()){
        return -1;
    }
    string temp;
    while(getline(file,temp)){
        seccomp_policy+=temp;
    }
    file.close();
    if(seccomp_policy.find("[pid]") != string::npos){
        seccomp_policy = seccomp_policy.replace( seccomp_policy.find("[pid]"),5,std::to_string(pid) );
    }
    struct sock_fprog prog;
    kafel_ctxt_t ctxt = kafel_ctxt_create();
    kafel_set_input_string(ctxt, seccomp_policy.c_str());
    if (kafel_compile(ctxt, &prog)) {
        fprintf(stderr, "policy compilation failed: %s", kafel_error_msg(ctxt));
        kafel_ctxt_destroy(&ctxt);
        return false;
    }
    kafel_ctxt_destroy(&ctxt);
    prctl(PR_SET_NO_NEW_PRIVS,1,0,0,0);
    prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog, 0, 0);
    free(prog.filter);
    return true;
}

int SubProcess::restrainSystemCall2(){
    DEBUG_PRINT(SCMP_SYS(execve));
    struct sock_filter execve_filter[] = {
            BPF_STMT(BPF_LD+BPF_W+BPF_ABS,0),
            BPF_JUMP(BPF_JMP+BPF_JEQ, SCMP_SYS(execve),0,1),
            BPF_STMT(BPF_RET+BPF_K,SECCOMP_RET_KILL),
            BPF_STMT(BPF_RET+BPF_K,SECCOMP_RET_ALLOW),
    };

    sock_fprog fprog = {
            (sizeof(execve_filter)/sizeof (execve_filter[0])),
            execve_filter
    };
    prctl(PR_SET_NO_NEW_PRIVS,1,0,0,0);
    if(prctl(PR_SET_SECCOMP,SECCOMP_MODE_FILTER,&fprog) != RV_OK){
        DEBUG_PRINT("load seccomp failed!");
    }

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
            return false;
        }
        if(dup2(fileno(outputFile),fileno(stdout)) == -1){
            DEBUG_PRINT("重定向错误！");
            return false;
        }
    }
    return true;
}