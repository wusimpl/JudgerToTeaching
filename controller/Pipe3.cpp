//
// Created by root on 2021/4/24.
//

#include "Pipe3.h"
#include "../common/Util.h"

int pipes[NUM_PIPES][2];


void closeAll(){
    for (auto & pipe : pipes) {
        for (int j : pipe) {
            close(j);
        }
    }
}


int pipe3(const string& cmd,Pipe3Run pipe3Run,void* args){
    if(pipe(pipes[STDIN]) == 0 && pipe(pipes[STDOUT]) == 0 && pipe(pipes[STDERR]) == 0){
        if(!fork()){ // subprocess
            dup2(pipes[STDIN][READ],STDIN_FILENO);
            dup2(pipes[STDOUT][WRITE],STDOUT_FILENO);
            dup2(pipes[STDERR][WRITE],STDERR_FILENO);

            closeAll();

            execl( "/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
        }else{ //parent process
            close(pipes[STDIN][READ]);
            close(pipes[STDOUT][WRITE]);
            close(pipes[STDERR][WRITE]);

            //此处本应放置与子进程进行管道交互的代码，但为了灵活性，这里的代码即使放到pipe3函数之后执行也能生效。
            // 这里给出的解决办法是讲代码用函数指针传进来并在此调用
            pipe3Run(args);
            closeAll();
            return RV_OK;
        }
    }else{
        printf("创建管道错误! errno code:%d\n",errno);
        return RV_ERROR;
    }
}

void pipeRunImpl(PipeArgs* args){
    pollfd fds[2] = {
            {pipes[STDOUT][READ],POLLREAD,0},
            {pipes[STDERR][READ],POLLREAD,0}
    };

    switch(poll(fds,2,5 * seconds)){
        case -1: //函数调用出错
            DEBUG_PRINT("poll error");
            break;
        case 0: //time out
            DEBUG_PRINT("poll time out");
            break;
        default: //得到数据返回
            switch (fds[0].revents) {
                case 0: // error
                    break;
                case POLLEND:
                    args->returnCode = STDOUT;
                    break;
                default:
                    readFromFD(fds[0].fd,args->stdOutput);
                    args->returnCode = STDOUT;
            }

            switch (fds[1].revents) {
                case 0: // error
                    break;
                case POLLEND:
                    break;
                default:
                    readFromFD(fds[1].fd,args->stdError);
                    args->returnCode = STDERR;
            }
    }
}
