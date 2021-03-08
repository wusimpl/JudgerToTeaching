#define DEBUG

#include "compiler/CCompiler.h"
#include <iostream>
using std::cout;
using std::endl;

int main() {

    JudgeConfig cfg;
    cfg.srcPath = "/root/main.cpp";

    CCompiler compiler(&cfg);
    Compiler::CompileResult compileResult = compiler.compile();
    cout << compileResult.compileOutput << endl;
    return 0;
}
