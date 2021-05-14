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


    string mkdir;
    string commandOutputInfo;
    /*
    if(execShellCommand(mkdir,commandOutputInfo) != 0){
        DEBUG_PRINT(commandOutputInfo);
        return;
    }*/

    //检查程序相关文件夹是否存在（需要用户自行根据config.ini的配置创建）
    if(execShellCommand("cd " + JudgeOutputFilePath,commandOutputInfo) != 0 &&
       execShellCommand("cd " + JudgeExeFilePath,commandOutputInfo) != 0){
        DEBUG_PRINT("找不到程序存放文件夹或(和)程序输出文件夹!");
        return;
    }

    //获取输出文件夹最大ID
    const char* parentDir = ".."; // 排除当前目录和父母目录
    const char* currentDir = ".";
    DIR* outputFileDir = nullptr;
    DIR* exeFileDir = nullptr;
    char outputPathMaxID[100] = {'0'};
    char exePathMaxID[100] = {'0'};
    struct dirent* ptr;

    outputFileDir=opendir(JudgeOutputFilePath.c_str());
    exeFileDir = opendir(JudgeExeFilePath.c_str());

    while ((ptr=readdir(outputFileDir))){
        if(ptr->d_type == DT_DIR){
            if(strcmp(ptr->d_name,parentDir) != 0
            && strcmp(ptr->d_name,currentDir) != 0 && strcmp(ptr->d_name,outputPathMaxID) > 0){
                strcpy(outputPathMaxID,ptr->d_name);
            }
        }
    }

    while ((ptr=readdir(exeFileDir))){
        if(ptr->d_type == DT_DIR){
            if(strcmp(ptr->d_name,parentDir) != 0
               && strcmp(ptr->d_name,currentDir) != 0 && strcmp(ptr->d_name,exePathMaxID) > 0){
                strcpy(exePathMaxID,ptr->d_name);
            }
        }
    }

    closedir(exeFileDir);
    closedir(outputFileDir);


    this->outputFilePath = JudgeOutputFilePath + std::to_string(atoi(outputPathMaxID)+1) + "/";
    DEBUG_PRINT(outputFilePath);
    this->exePath = JudgeExeFilePath + std::to_string(atoi(exePathMaxID)+1);
    mkdir = string("mkdir -p ").append(outputFilePath).append(" 2>&1");
    if(execShellCommand(mkdir,commandOutputInfo) != 0){
        DEBUG_PRINT(commandOutputInfo);
        return;
    }


    sysCallFilterMode = WHITE_LIST_MODE;//默认白名单模式
    if(filterMode == "b"){
        sysCallFilterMode = BLACK_LIST_MODE;
    }else if(filterMode == "w"){

    }else{
        DEBUG_PRINT("系统调用配置出错!已默认为白名单模式");
    }

}

JudgeConfig::JudgeConfig() {
    compileMethod=1;
    fileType = SourceFileType::cpp;
    for (int & i : sysCallList) {
        i = -1;
    }
    for (auto & programArg : programArgs) {
        programArg = nullptr;
    }
}

JudgeConfig::~JudgeConfig() {
    for (auto & programArg : programArgs) {
        delete programArg;
    }
}
