//
// Created by root on 2021/4/25.
//

#ifndef JUDGERTOTEACHING_MACRO_H
#define JUDGERTOTEACHING_MACRO_H

#include <string>
#include <fstream>
#include <iostream>
#include <ios>
#include <cstring>
#include <map>
#include <dirent.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <ctime>
#include <sys/resource.h>

using std::string;
using std::fstream;
using std::getline;
using std::vector;
using std::cout;
using std::endl;


#define DEBUG_PRINT(x) cout<<x<<endl; // debug 打印调试输出
#define MAX_SYSCALL_NUMBER 256 // 系统调用配置表最大长度
#define BLACK_LIST_MODE 1 // 系统调用黑名单模式
#define WHITE_LIST_MODE 0 // 系统调用白黑名单模式
#define KB (1024)
#define MB (KB*1024)
#define seconds (1000)
#define minutes (seconds*60)
#define UNLIMITED (-1) //资源无限制
#define MAX_PROGRAM_ARGS 100 // 用户程序命令行参数最大数量
#define RV_ERROR (-1) // 通用错误返回状态码
#define RV_OK (0) // 通用正确返回状态码

// 加载一条配置
#define LOAD_ONE_CONFIG(key,value) \
 string value; \
 if( !configurationTool.getValue(key,value) ){ \
    DEBUG_PRINT("加载单项配置错误!");        \
    configPath = "error";              \
    return;\
 } \

// 设置资源限制
#define SetDefaultResourceLimit(var,value) \
 this->requiredResourceLimit.var = value; \

#define MAX_TEST_FILE_NUMBER 20 // 测试数据最大数量
#define KILL_PROCESS(pid) (kill(pid,SIGKILL))  // 用于杀死进程的宏
#define KILLED_SUCCESS nullptr
#define KILLED_ERROR_INFO (&errno)

#define TIME_VALUE(timeval) (timeval.tv_sec*1000.0 + timeval.tv_usec/1000.0) // ms
#define R_CPU_TIME(rusage) (TIME_VALUE(rusage.ru_utime) + TIME_VALUE(rusage.ru_stime)) // ms

/**
 * 限制进程各类资源的使用的宏
 */
typedef struct rlimit rlimit;
#define SetRLimit_X(X,rlimitStruct) setrlimit(RLIMIT_##X,&(rlimitStruct))

#endif //JUDGERTOTEACHING_MACRO_H
