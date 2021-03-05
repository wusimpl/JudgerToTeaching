//
// Created by rory on 2021/3/5.
//
/*
 * 编译类
 */
#ifndef JUDGERTOTEACHING_COMPILER_H
#define JUDGERTOTEACHING_COMPILER_H

#include "../utils/Data.h"
#include <sstream>
using std::stringstream;

class Compiler {
protected:
    JudgeConfig* cfg{};

public:
    virtual string generateCompileCommand()=0;
};

class CCompiler : public Compiler {
    string generateCompileCommand() override{
        if (!cfg){
            return string("nullptr");
        }
        stringstream ss;
        ss << "gcc "<<cfg->srcPath<<"-o "<<cfg->exePath;
        return ss.str();
    }
};


#endif //JUDGERTOTEACHING_COMPILER_H
