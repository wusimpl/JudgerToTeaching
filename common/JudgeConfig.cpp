//
// Created by root on 2021/4/25.
//

#include "JudgeConfig.h"
#include "ConfigurationTool.h"


JudgeConfig::JudgeConfig(string& configPath){

    //
    fileType = SourceFileType::none;
    for (int & i : sysCallList) {
        i = -1;
    }
    for (auto & programArg : programArgs) {
        programArg = nullptr;
    }

    string currentTime = getCurrentFormattedTime();

    //读取配置文件
    ConfigurationTool configurationTool{};
    bool success = configurationTool.load(configPath);
    if(!success){
        DEBUG_PRINT("配置文件加载错误!\n");
        exit(RV_ERROR);
    }
    LOAD_ONE_CONFIG("JudgeOutputFilePath",JudgeOutputFilePath); // 程序输出文件存放路径
    LOAD_ONE_CONFIG("JudgeExeFilePath",JudgeExeFilePath); // 程序存放路径
    LOAD_ONE_CONFIG("DefaultLimitedCPUTime",DefaultLimitedCPUTime);
    LOAD_ONE_CONFIG("SysCallFilterMode",filterMode); //系统调用过滤模式
    SetDefaultResourceLimit(cpuTime, atoi(DefaultLimitedCPUTime.c_str()));
    LOAD_ONE_CONFIG("DefaultLimitedRealTime",DefaultLimitedRealTime);
    SetDefaultResourceLimit(realTime, atoi(DefaultLimitedRealTime.c_str()));
    LOAD_ONE_CONFIG("DefaultLimitedStack",DefaultLimitedStack);
    SetDefaultResourceLimit(stack, atoi(DefaultLimitedStack.c_str()));
    LOAD_ONE_CONFIG("DefaultLimitedOutputSize",DefaultLimitedOutputSize);
    SetDefaultResourceLimit(outputSize, atoi(DefaultLimitedOutputSize.c_str()));
    LOAD_ONE_CONFIG("DefaultLimitedMemory",DefaultLimitedMemory);
    SetDefaultResourceLimit(memory, atoi(DefaultLimitedMemory.c_str()));


    //创建临时工作目录
    string homeDirectory = getHomeDirectory();
    JudgeOutputFilePath = homeDirectory + JudgeOutputFilePath;
    JudgeExeFilePath = homeDirectory + JudgeExeFilePath;
    string mkdir = string("mkdir -p ").append(JudgeExeFilePath).append(" 2>&1");
    string commandOutputInfo;
    if(execShellCommand(mkdir,commandOutputInfo) != 0){
        DEBUG_PRINT(commandOutputInfo);
        return;
    }

    this->outputFilePath = string(currentTime).insert(0,JudgeOutputFilePath).append("-out/");
    mkdir = string("mkdir -p ").append(outputFilePath).append(" 2>&1");
    if(execShellCommand(mkdir,commandOutputInfo) != 0){
        DEBUG_PRINT(commandOutputInfo);
        return;
    }
    this->exePath = currentTime.insert(0,JudgeExeFilePath).append(".executable");

    sysCallFilterMode = WHITE_LIST_MODE;//默认白名单模式
    if(filterMode == "b"){
        sysCallFilterMode = BLACK_LIST_MODE;
    }else if(filterMode == "w"){

    }else{
        DEBUG_PRINT("系统调用配置出错!设置为白名单模式已默认");
    }

}