//
// Created by rory on 2021/3/6.
//

#ifndef JUDGERTOTEACHING_PROGRAMCONTROLLER_H
#define JUDGERTOTEACHING_PROGRAMCONTROLLER_H

#include "../utils/util.h"
#include <csignal>
#include <pthread.h>
#include <cerrno>
#include <unistd.h>

#include <sys/wait.h>
#include <ctime>
#include <sys/resource.h>
#include <sys/types.h>

/**
 * 限制进程各类资源的使用的宏
 */
typedef struct rlimit rlimit;
#define SetRLimit_X(X,rlimitStruct) setrlimit(RLIMIT_##X,&rlimitStruct)
/**
 * rlimit_X(AS,rlimit)
 * rlimit_X(CORE,rlimit)
 * rlimit_X(CPU,rlimit)
 * rlimit_X(DATA,rlimit)
 * rlimit_X(FSIZE,rlimit)
 * rlimit_X(NOFILE,rlimit)
 */


/**
 * 子进程控制器，负责运行、监控被测代码子进程
 */
class ProcessController {
private:
    JudgeConfig* config;

public:

    enum ResultCode{
        OK, //运行成功
        PERMISSION_DENIED, //权限不够，请尝试提权运行
        FORK_ERROR, //创建子进程失败
        EXEC_ERROR, //
    };

    explicit ProcessController(JudgeConfig* cfg):config(cfg){
    }
    /**
     * 运行config中的exe文件
     * @return 结果状态码，详细返回值定义请查看枚举定义
     */
    ResultCode run();

    JudgeConfig *getConfig() const {
        return config;
    }

    void setConfig(JudgeConfig *cfg) {
        ProcessController::config = cfg;
    }
};

class SubProcess{



};
#endif //JUDGERTOTEACHING_PROGRAMCONTROLLER_H
