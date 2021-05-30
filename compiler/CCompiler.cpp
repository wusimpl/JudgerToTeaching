//
// Created by rory on 2021/3/7.
//

#include "CCompiler.h"
#include "../controller/Pipe3.h"
#include "../common/Util.h"

#include <sstream>
using std::stringstream;

string CCompiler::generateCompileCommand() const {
    if (!cfg){
        return string("nullptr");
    }
    stringstream ss;
    ss << "gcc " << cfg->codePath << " -o " << cfg->exePath;
    return ss.str();
}


CCompiler::CCompiler(JudgeConfig *config) : Compiler(config) {

}

CompileResult CCompiler::compile() {
    CompileResult compileResult;
    string cmd = generateCompileCommand();
    PipeArgs args = {};
//    int returnValue = system(cmd.c_str()); // 0:成功 非0:失败
    int returnValue = execShellCommandPlus(cmd,static_cast<void*>(&args));//执行编译命令

//    bool mainExists = false;
//    if(cfg->compileMethod == 2){ //make:检查是否生成了main文件
//        Dir dir = getFilesOfDirWithFullPath(cfg->exePath);
//        for (int i = 0; i < dir.size; ++i) {
//            if(dir.files[i] == "main"){
//                mainExists = true;
//            }
//        }
//
//        if(!mainExists){
//            compileResult.status = CompileResult ::CompileStatus::CE;
//            compileResult.compileOutput = "找不到main文件!请检查makefile文件是否编写正确！";
//            return compileResult;
//        }
//    }


    if(returnValue == RV_OK){
        switch (args.returnCode) {
            case STDOUT:
                compileResult.status = CompileResult ::CompileStatus::OK;
                compileResult.compileOutput = args.stdOutput;
                break;
            case STDERR:
                compileResult.status = CompileResult ::CompileStatus::CE;
//                cfg->wholeResult.errorCode = WholeResult::CE;
                compileResult.compileOutput = args.stdError;
                break;
        }
    }else{
        compileResult.status = CompileResult::CompileStatus::SE;
        compileResult.errnoValue = errno;
    }

    return compileResult;
}
