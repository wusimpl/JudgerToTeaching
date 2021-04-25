//
// Created by rory on 2021/3/6.
//

#ifndef JUDGERTOTEACHING_PROGRAMCONTROLLER_H
#define JUDGERTOTEACHING_PROGRAMCONTROLLER_H

#include "../common//Util.h"
#include "../common/JudgeConfig.h"
#include <csignal>
#include <cerrno>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * 子进程控制器，负责运行、监控被测代码子进程
 */
class ProcessController {
private:
    JudgeConfig* config{};

public:

    /**
     * 子线程需要的参数信息
     */
    typedef struct ThreadInfo{
        pid_t pid;
        double timeout;
    }ThreadInfo;

    explicit ProcessController(JudgeConfig* cfg):config(cfg){}

    /**
     * 进程超时监视器，若子进程运行超时，则发送信号杀死
     * @param timeout_killer_args
     * @return
     */
    static void* timeoutKiller(void* threadInfo);

    /**
     * 进程状态跟踪器，拦截子进程的exit系统调用，获取最后一刻的/proc/[subpid]/status
     * (!!! deprecated,switching to trace subprocess in main thread)
     * @param threadInfo
     * @return
     */
//    static void* statusTracer(void* threadInfo);

    /**
     * 运行config中的exe文件
     * @return 结果状态码，详细返回值定义请查看枚举定义
     */
    ControllerResult run();
};


#endif //JUDGERTOTEACHING_PROGRAMCONTROLLER_H
