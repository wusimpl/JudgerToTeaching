//
// Created by root on 2021/4/24.
//

#ifndef JUDGERTOTEACHING_PIPE3_H
#define JUDGERTOTEACHING_PIPE3_H

#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <cstdlib>
#include <string>
using std::string;


#define NUM_PIPES 3 // 三条管道：for stdin,stdout,stderr
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define READ 0
#define WRITE 1

/**
 * see details in pipe3 function's comment
 */
typedef void (*Pipe3Run)(void*);

extern int pipes[NUM_PIPES][2];

/**
 * 一口气关闭所有管道口
 * @param pipes
 */
void closeAll();

/**
 * 建立管道 + 重定向
 * @param argv 对端命令行参数(example : argv={"/usr/bash","-c","echo hello world"})
 * @param Pipe3Run
 */
int pipe3(const string& cmd,Pipe3Run pipe3Run,void* args);

#endif //JUDGERTOTEACHING_PIPE3_H
