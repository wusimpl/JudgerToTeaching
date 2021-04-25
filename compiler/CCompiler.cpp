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

Compiler::CompileResult CCompiler::compile() {
    CompileResult compileResult;
    string cmd = generateCompileCommand();
    PipeArgs args = {};

    int returnValue = execShellCommandPlus(cmd,args);//执行编译命令
    if(returnValue == RV_OK){
        switch (args.returnCode) {
            case STDOUT:
                compileResult.status = CompileResult ::CompileStatus::OK;
                break;
            case STDERR:
                compileResult.status = CompileResult ::CompileStatus::CE;
                break;
        }
    }else{
        compileResult.status = CompileResult::CompileStatus::SE;
        compileResult.errnoValue = errno;
    }

    return compileResult;
}
