//
// Created by WilliamsAndy on 2021/4/7.
//

#include "CXXCompiler.h"
#include <sstream>
using std::stringstream;

string CXXCompiler::generateCompileCommand() const {
    if (!cfg){
        return string("nullptr");
    }
    stringstream ss;
    ss << "g++ "<<"-Wall -lm "<<cfg->srcPath<<" -o "<<cfg->exePath;
    return ss.str();
}

CXXCompiler::CXXCompiler(JudgeConfig *config) : CCompiler(config) {

}
