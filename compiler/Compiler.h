//
// Created by rory on 2021/3/5.
//
/*
 * 编译类
 */
#ifndef JUDGERTOTEACHING_COMPILER_H
#define JUDGERTOTEACHING_COMPILER_H

#include "../common/JudgeConfig.h"
#include "../common/Result.h"

class Compiler {
protected:
    JudgeConfig* cfg{};
public:
    explicit Compiler(JudgeConfig* config){
        this->cfg = config;
    }

    virtual ~Compiler()= default;
    /**
     * 编译源文件，若编译成功，cfg的exePath会被赋值
     * @param compileInfo 编译信息会输出到此参数
     * @return 编译结果，具体含义请查看结构体定义
     */
    virtual  CompileResult compile() = 0;

protected:
    /**
     * 生成相关编译器编译命令
     * @return 字符串形式的编译命令
     */
    virtual string generateCompileCommand() const =0;

};

#endif //JUDGERTOTEACHING_COMPILER_H
