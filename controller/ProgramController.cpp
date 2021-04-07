//
// Created by rory on 2021/3/6.
//

#include "ProgramController.h"

ProcessController::ResultCode ProcessController::run() {
    //检查root权限
    if(getuid() != 0){
        return PERMISSION_DENIED;
    }

    //
    int subPid = vfork();
    if (subPid > 0){//父进程代码
        rlimit as{};
        SetRLimit_X(AS,as);
    }else if(subPid == 0){//子进程代码

    }else{
        return FORK_ERROR;
    }

    return OK;
}
