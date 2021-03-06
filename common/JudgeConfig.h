//
// Created by root on 2021/4/25.
//

#ifndef JUDGERTOTEACHING_JUDGECONFIG_H
#define JUDGERTOTEACHING_JUDGECONFIG_H

#include "Macro.h"
#include "../common/Util.h"
#include "../common/Result.h"

/*
 * 判题配置
 */
class JudgeConfig{
public:
    explicit JudgeConfig(string& configPath);
    explicit JudgeConfig();

    ~ JudgeConfig();

    string codePath; //源代码文件全路径
    string exePath; //可执行文件全路径
    string testInPath; //测试输入文件路径(文件夹)，每组测试数据都要求与输出文件名配对
    string testOutPath; //测试输出文件路径(文件夹)
    char* programArgs[100]{}; //被测试程序的参数 一定要初始化为nullptr,execve要求argv最后一个参数为nullptr

    int compileMethod; // 编译方式
    int compareMethod; //答案比较方式

    string outputFilePath; //输出文件路径(重定向被测程序的stdout到outputFilePath)
    SourceFileType fileType; //源文件类型，帮助调用相应编译器

    int sysCallList[MAX_SYSCALL_NUMBER]{}; //系统调用名单，与filterMode搭配使用
    int sysCallFilterMode=1; //1：黑名单模式 0：白名单模式
    string seccompFilterFilePath; //系统调用配置文件

    ResourceLimit requiredResourceLimit; //题目要求的资源限制值
    ResourceLimit usedResourceLimit; //程序实际所用资源
};

#endif //JUDGERTOTEACHING_JUDGECONFIG_H
