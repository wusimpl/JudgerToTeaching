//
// Created by rory on 2021/3/7.
//

#include "CCompiler.h"

#include <sstream>
using std::stringstream;

string CCompiler::generateCompileCommand() const {
    if (!cfg){
        return string("nullptr");
    }
    stringstream ss;
    ss << "gcc "<<"-Wall -lm "<<cfg->srcPath<<" -o "<<cfg->exePath;
    return ss.str();
}


CCompiler::CCompiler(JudgeConfig *config) : Compiler(config) {

}

Compiler::CompileResult CCompiler::compile() const {
    
    CompileResult compileResult;
    int errnoValue = execShellCommand(generateCompileCommand()+" 2>&1",compileResult.compileOutput);//执行编译命令
    if(errnoValue != 0){//执行错误
        compileResult.status = CompileResult::CompileStatus::SE;
        compileResult.errnoValue = errnoValue;
    }else{
        string lastCommandResult;
        execShellCommand("echo $? 2>&1",lastCommandResult);//查看是否编译成功
        if(lastCommandResult == "0\n"){//0表示上一条命令执行成功
            compileResult.status = Compiler::CompileResult::OK;
        }else{
            compileResult.status = Compiler::CompileResult::CE;
        }
    }

    return compileResult;
}
