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
    Dir dir = getFilesOfDirWithFullPath(cfg->codePath);
    switch (cfg->compileMethod) {
        case 1: // 普通编译
            ss <<"/usr/bin/g++ -w -std=c++11 ";
            for (int i = 0; i < dir.size; ++i) {
                if( dir.files[i].find(".cpp")!=string::npos ||
                    dir.files[i].find(".c")!=string::npos){
                    ss<<dir.files[i]<<" ";
                }
            }
            ss<<" -o "<< cfg->exePath<<"main"; //二进制文件默认为main
            break;
        case 2: // makefile.txt
            ss << "cd "<<cfg->codePath<<" && "; // 必须是codePath而非exePath，make会在当前目录寻找源文件
            for (int i = 0; i < dir.size; ++i) {
                if(dir.files[i].find("makefile.txt") != string::npos){
                    DEBUG_PRINT(dir.files[i]);
                    ss << "/usr/bin/make -f "<<dir.files[i];
                    break;
                }
            }
            ss << " && "<<"mv " << cfg->codePath << "main " << cfg->exePath <<"main";
            break;
        case 3: //CMakeLists.txt

            break;
    }
    DEBUG_PRINT("编译命令：" << ss.str());
    return ss.str();
}

CXXCompiler::CXXCompiler(JudgeConfig *config) : CCompiler(config) {

}
