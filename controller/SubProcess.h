//
// Created by WilliamsAndy on 2021/4/9.
//

#ifndef JUDGERTOTEACHING_SUBPROCESS_H
#define JUDGERTOTEACHING_SUBPROCESS_H
#include "../utils/util.h"
#include <sys/resource.h>
#include <seccomp.h>

class SubProcess{
private:
    JudgeConfig* config;
    FILE* openedReadFiles[MAX_TEST_FILE_NUMBER]{nullptr};
    FILE* openedWriteFiles[MAX_TEST_FILE_NUMBER]{nullptr};
    scmp_filter_ctx ctx; // 系统调用过滤规则
public:
    explicit SubProcess(JudgeConfig* cfg);
    ~SubProcess();

    void runUserProgram();

private:

    /**
    * 设置资源限制的函数
    */
    void setResourceLimit();

    /**
     * 限制系统调用
     * @return
     */
    bool restrainSystemCall();

    void redirectIO();
};


#endif //JUDGERTOTEACHING_SUBPROCESS_H
