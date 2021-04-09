#include "compiler/CCompiler.h"
#include "compiler/CXXCompiler.h"
#include "controller/ProgramController.h"
#include <iostream>
using std::cout;
using std::endl;



int main(int argc,char* argv[]) {

    string configPath;
    execShellCommand("whoami",configPath);
    configPath = "/home/" + configPath.replace(configPath.find('\n'),1,"") + "/.ets/judge/config.ini";
    JudgeConfig cfg(configPath);
    cfg.srcPath = "/home/andy/main.cpp";
    cfg.fileType = getExternalName(cfg.srcPath);
    cfg.testInPath = "/home/andy/in/";
    cfg.testOutPath = "/home/andy/out/";
    cfg.programArgs[0] = "a.out";
    cfg.programArgs[1] = "okay";
    Compiler* compiler = nullptr;

    switch (cfg.fileType) {
        case none:
            break;
        case c:
            break;
        case cpp:
            compiler = new CXXCompiler(&cfg);
            break;
        case java:
            break;
        case py:
            break;
    }
    Compiler::CompileResult compileResult = compiler->compile();
    if(compileResult.status == Compiler::CompileResult::OK){
        delete compiler;

        ProcessController controller(&cfg);
        controller.run();
    } else{
        DEBUG_PRINT("编译错误！");
    }
    return 0;
}
