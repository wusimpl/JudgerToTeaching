//
// Created by WilliamsAndy on 2021/4/9.
//

#ifndef JUDGERTOTEACHING_SUBPROCESS_H
#define JUDGERTOTEACHING_SUBPROCESS_H
#include "../common/Util.h"
#include "../common/JudgeConfig.h"
#include <sys/resource.h>
#include <seccomp.h>

class SubProcess{
private:
    JudgeConfig* config;
    FILE* openedReadFiles[MAX_TEST_FILE_NUMBER]{nullptr};
    FILE* openedWriteFiles[MAX_TEST_FILE_NUMBER]{nullptr};
    scmp_filter_ctx ctx; // 系统调用过滤规则
    int pid = -1;
    int initSuccess = true;
public:
    explicit SubProcess(JudgeConfig* cfg,int pid);
    ~SubProcess(); // 因为execve成功执行时不会返回,所以这破函数基本没啥用

    static void xcpuSignalHandler(int sig);

    /**
     * 已做好，前期准备工作，正式运行用户程序
     */
    int runUserProgram();

private:

    /**
    * 设置资源限制的函数
    */
    int setResourceLimit();

    /**
     * 限制系统调用
     * @return
     */
    int restrainSystemCall();

    int redirectIO();
};


#endif //JUDGERTOTEACHING_SUBPROCESS_H
