//
// Created by root on 2021/5/12.
//

#ifndef JUDGERTOTEACHING_SECCOMPRULES_H
#define JUDGERTOTEACHING_SECCOMPRULES_H
#include "../common/JudgeConfig.h"

#define LOAD_SECCOMP_FAILED (-1)

int c_cpp_seccomp_rules(JudgeConfig* config, bool allow_write_file);

#endif //JUDGERTOTEACHING_SECCOMPRULES_H
