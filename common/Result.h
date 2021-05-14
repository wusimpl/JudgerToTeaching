//
// Created by root on 2021/4/25.
//

#ifndef JUDGERTOTEACHING_RESULT_H
#define JUDGERTOTEACHING_RESULT_H
#include "Macro.h"
#include "Util.h"

// used when called by Java Native Interface
#define COMPILE_OK 0
#define COMPILE_ERROR 1
#define SYSTEM_ERROR 2

/**
 *保存编译层信息的结构体，编译时发生系统错误时(SE)，错误代号errno在errnoValue中
 */
typedef struct CompileResult{
    enum CompileStatus{
        OK=0,//编译成功
        CE=1, //编译失败
        SE=2//发生系统错误，原因请查看errno变量
    };
    int errnoValue; //保存错误信息,0 means no error
    string compileOutput;//编译命令输出信息
    CompileStatus status;
    CompileResult(){
        errnoValue = 0;//default value,means no error
        status = CompileStatus::OK;
    }
}CompileResult;

/**
 * 保存运行层信息结构体
 */
typedef struct ControllerResult{
    enum RunStatus{
        EXITED_NORMALLY=0, //运行成功
        PERMISSION_DENIED=1, //权限不够，请尝试提权运行
        FORK_ERROR=2, //创建子进程失败
        TIME_OUT=3, // 运行超时
        THREAD_CREATE_ERROR=4, //线程创建失败
        WAIT_ERROR=5, //wait子进程失败
    }runStatus;

    /**
    *-1: UN  // unknown
    * 0: OK
    * 1: SE
    * 2: CLE, //Cpu Time Limited Error
    * 3: RLE, //Real Time Limited Error
    * 4: MLE, //Memory Limited Error
    * 5: OLE, //Output Limited Error, 输出文件的数据过大
    * 6: RE, //Runtime Error
    */
    int status;
    int returnValue; // 用户程序返回值
    ResourceLimit usedResourceLimit;

}ControllerResult;


/*
**
* 整个判题流程的评判结果信息
*/
typedef struct WholeResult{
    enum ErrorCode{
        NONE=-1, //初始状态
        CE=0, //Compile Error

        CLE, //Cpu Time Limited Error
        RLE, //Real Time Limited Error
        MLE, //Memory Limited Error
        OLE, //Output Limited Error, 输出文件的数据过大
        RE, //Runtime Error
        RF, //Restricted Function, 调用了危险的函数

        WA, //Wrong Answer
        PC, //Partially Correct, 通过了部分测试数据
        AC, //Accepted
        PE, //Presentation Error, 输出格式错误(可能是空格、换行、数值精度等未控制好)

        SE //System Error, 评判系统内部错误

    }errorCode;

    CompileResult compileResult{};
    ControllerResult controllerResult{};

    WholeResult(){
        errorCode = NONE;
    }

}WholeResult;


#endif //JUDGERTOTEACHING_RESULT_H
