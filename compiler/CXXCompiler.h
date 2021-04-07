//
// Created by WilliamsAndy on 2021/4/7.
//

#ifndef JUDGERTOTEACHING_CXXCOMPILER_H
#define JUDGERTOTEACHING_CXXCOMPILER_H

#include "CCompiler.h"

class CXXCompiler : public CCompiler{
public:
    explicit CXXCompiler(JudgeConfig *config);

protected:
    string generateCompileCommand() const override;
};


#endif //JUDGERTOTEACHING_CXXCOMPILER_H
