#include "compiler/CCompiler.h"
#include "compiler/CXXCompiler.h"
#include "controller/ProgramController.h"
#include <iostream>
using std::cout;
using std::endl;

int main(int argc,char* argv[]) {
    // 权限检查
    if(!isRoot()){
        DEBUG_PRINT("无root权限!");
        return RV_ERROR;
    }
    //初始化配置文件
    string configPath;
    configPath = getHomeDirectory() + ".ets/judge/config.ini";
    JudgeConfig cfg(configPath);
    if(configPath == string("error")){
        return RV_ERROR;
    }
    cfg.srcPath = "/root/test/test.cpp";
    cfg.fileType = getExternalName(cfg.srcPath);
    cfg.testInPath = "/root/test/in/";
    cfg.testOutPath = "/root/test/out/";
//    cfg.sysCallList[0] = SCMP_SYS(open);
    //编译
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
    // "gcc main.c -o main" => ["gcc","main.c","-o","main"]
    if(compileResult.status == Compiler::CompileResult::OK){
        delete compiler;
    // 运行
        ProcessController controller(&cfg);
        controller.run();
    } else{
        DEBUG_PRINT("编译错误！");
    }
    return 0;
}
