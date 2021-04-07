#define DEBUG

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
    } else{

    }
    return 0;
}
