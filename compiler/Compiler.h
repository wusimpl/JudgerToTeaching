//
// Created by rory on 2021/3/5.
//
/*
 * 编译类
 */
#ifndef JUDGERTOTEACHING_COMPILER_H
#define JUDGERTOTEACHING_COMPILER_H

#include "../utils/util.h"

class Compiler {
protected:
    JudgeConfig* cfg{};
public:

    /**
     *保存编译结果的结构体，编译时发生系统错误时(SE)，错误代号errno在errnoValue中
     */
    typedef struct CompileResult{
        enum CompileStatus{
            OK,//编译成功
            SE,//发生系统错误，原因请查看errno变量
            CE //编译失败
        };
        int errnoValue; //保存错误信息,0 means no error
        string compileOutput;//编译命令输出信息
        CompileStatus status;
        CompileResult(){
            errnoValue = 0;//default value,means no error
            status = CompileStatus::OK;
        }

    }CompileResult;

    explicit Compiler(JudgeConfig* config){
        this->cfg = config;
    }

    virtual ~Compiler()= default;
    /**
     * 编译源文件，若编译成功，cfg的exePath会被赋值
     * @param compileInfo 编译信息会输出到此参数
     * @return 编译结果，具体含义请查看结构体定义
     */
    virtual  CompileResult compile() const = 0;

protected:
    /**
     * 生成相关编译器编译命令
     * @return 字符串形式的编译命令
     */
    virtual string generateCompileCommand() const =0;

};

#endif //JUDGERTOTEACHING_COMPILER_H
