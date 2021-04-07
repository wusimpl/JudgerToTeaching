//
// Created by rory on 2021/3/6.
//

#include "ProgramController.h"

ProcessController::ControllerResult ProcessController::run() {
    ControllerResult controllerResult;
    //检查root权限
    if(getuid() != 0){
        controllerResult.runStatus = ControllerResult::PERMISSION_DENIED;
        return controllerResult;
    }

    //
    int subPid = vfork();
    if (subPid > 0){//父进程代码
        rlimit as{};
        SetRLimit_X(AS,as);
    }else if(subPid == 0){//子进程代码

    }else{
        controllerResult.runStatus = ControllerResult::FORK_ERROR;
        return controllerResult;
    }

    controllerResult.runStatus = ControllerResult::OK;
    return controllerResult;
}
