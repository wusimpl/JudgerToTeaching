//
// Created by rory on 2021/3/6.
//

#ifndef JUDGERTOTEACHING_PROGRAMCONTROLLER_H
#define JUDGERTOTEACHING_PROGRAMCONTROLLER_H

#include "../utils/util.h"
#include <csignal>
#include <cerrno>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>



/**
 * 子进程控制器，负责运行、监控被测代码子进程
 */
class ProcessController {
private:
    JudgeConfig* config;

public:

    typedef struct ControllerResult{
        enum RunStatus{
            EXITED_NORMALLY, //运行成功
            PERMISSION_DENIED, //权限不够，请尝试提权运行
            FORK_ERROR, //创建子进程失败
            WAIT_ERROR, // 阻塞失败
            THREAD_CREATE_ERROR //线程创建失败
        }runStatus;
    }ControllerResult;

    typedef struct ThreadInfo{
        pid_t pid;
        unsigned long timeout;
    }ThreadInfo;

    explicit ProcessController(JudgeConfig* cfg):config(cfg){}

    /**
     * 进程超时监视器，若子进程运行超时，则发送信号杀死
     * @param timeout_killer_args
     * @return
     */
    static void* timeoutKiller(void* threadInfo);


    /**
     * 运行config中的exe文件
     * @return 结果状态码，详细返回值定义请查看枚举定义
     */
    ControllerResult run();

    JudgeConfig *getConfig() const {
        return config;
    }

    void setConfig(JudgeConfig *cfg) {
        ProcessController::config = cfg;
    }
};


#endif //JUDGERTOTEACHING_PROGRAMCONTROLLER_H
