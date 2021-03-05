//
// Created by rory on 2021/3/5.
//

#ifndef JUDGERTOTEACHING_DATA_H
#define JUDGERTOTEACHING_DATA_H

#include <ctime>
#include "util.h"
#include <cstring>

/*
 * 判题配置
 */
typedef struct JudgeConfig{
    JudgeConfig(){
        srcPath = exePath = outputFilePath = nullptr;
        sysCallList = nullptr;

        string currentTime = getCurrentFormattedTime();
        const char* timePath = string(currentTime).insert(1,"/tmp/judgeoutput/").append(".out").c_str();
        strcpy(outputFilePath,timePath);
        timePath = currentTime.insert(1,"/tmp/judgeexeoutput/").append(".out").c_str();
        strcpy(exePath,timePath);

        filterMode = true;
        limitedCPUTime = 1500;
        limitedMemory = 5000;
    }

    char* srcPath; //源代码文件路径
    char* exePath; //可执行文件路径
    char* outputFilePath; //输出文件路径(重定向被测程序的stdout到outputFilePath)

    int* sysCallList; //系统调用名单，与filterMode搭配使用
    bool filterMode; //true：白名单模式 false：黑名单模式

    int limitedCPUTime; // 单位为 ms
    int limitedMemory; // 单位为 KB
}JudgeConfig;

/**
 * 评判结果
 */
enum JudgeResult{
    CE=0, //Compile Error

    TLE, //Time Limited Error
    MLE, //Memory Limited Error
    OLE, //Output Limited Error, 通常意味着短时间内输出过多
    RE, //Runtime Error
    RF, //Restricted Function, 调用了危险的函数

    WA, //Wrong Answer
    PC, //Partially Correct, 通过了部分测试数据
    AC, //Accepted
    PE, //Presentation Error, 输出格式错误(可能是空格、换行、数值精度等未控制好)

    SE //System Error, 评判系统内部错误
};

/*
 *
 */
struct JudgeInfo{
    int usedCPUTime; //被测程序cpu使用时间
    int usedMemory;

};
#endif //JUDGERTOTEACHING_DATA_H
