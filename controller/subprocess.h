//
// Created by root on 2021/4/23.
//

#ifndef JUDGERTOTEACHING_SUBPROCESS_H
#define JUDGERTOTEACHING_SUBPROCESS_H

#include "../utils/util.h"
#include <sys/resource.h>
#include <seccomp.h>


extern FILE* openedReadFiles[MAX_TEST_FILE_NUMBER];
extern FILE* openedWriteFiles[MAX_TEST_FILE_NUMBER];
extern scmp_filter_ctx ctx; // 系统调用过滤规则

void subProcessInit(JudgeConfig* cfg);

void runUserProgram(JudgeConfig *config);


/**
 * 设置资源限制的函数
 * @param config
 */
void setResourceLimit(JudgeConfig *config);

/**
 * 限制系统调用
 * @return
 */
bool restrainSystemCall(JudgeConfig *config);

/**
 *
 * @param config
 */
void redirectIO(JudgeConfig *config);


#endif //JUDGERTOTEACHING_SUBPROCESS_H
