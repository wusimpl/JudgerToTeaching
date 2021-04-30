#include "compiler/CCompiler.h"
#include "compiler/CXXCompiler.h"
#include "controller/ProgramController.h"
#include "checker/AnswerChecker.h"
#include <seccomp.h>


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
    cfg.srcPath = "/root/test/testprintf.cpp";
    cfg.fileType = getExternalName(cfg.srcPath);
    cfg.testInPath = "/root/test/in/";
    cfg.testOutPath = "/root/test/out/";
    cfg.sysCallList[0] = SCMP_SYS(open);
    cfg.sysCallList[1] = SCMP_SYS(read);


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
    CompileResult compileResult = compiler->compile();
    if(compileResult.status == CompileResult::OK){
        delete compiler;
        // 运行
        ProcessController controller(&cfg);
        ControllerResult controllerResult;
        controllerResult = controller.run();
        if(controllerResult.runStatus == ControllerResult::RunStatus::EXITED_NORMALLY){
            //答案对比
            if(!cfg.testOutPath.empty()){

                AnswerChecker checker(cfg.testOutPath,cfg.outputFilePath);
//                AnswerChecker::compareByByte("/root/test/output/1.out","/root/test/out/1.out");
                checker.compareByTextByLine();
            }
        }

    } else{
        DEBUG_PRINT("编译错误！");
        DEBUG_PRINT(compileResult.compileOutput);
    }
    return 0;
}
