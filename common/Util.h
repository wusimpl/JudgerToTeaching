//
// Created by rory on 2021/3/5.
//

#ifndef JUDGERTOTEACHING_UTIL_H
#define JUDGERTOTEACHING_UTIL_H

#include "Macro.h"

/**
 * 源代码类型
 */
enum SourceFileType{
    none=0,
    c,
    cpp,
    java,
    py,
};

/**
 * 目录结构体，存放目录中的所有文件名
 */
typedef struct Dir{
    string dirPath;
    string* files;
    int size;

    Dir(){
        files = nullptr;
        size= 0;
    }

    Dir(string directory,int s){
        dirPath = std::move(directory);
        size = s;
        files = new string[size];
    }

    Dir(const Dir& dir){
        this->size = dir.size;
        this->dirPath = dir.dirPath;
        files = new string[size];
        for (int i = 0; i < size; ++i) {
            files[i] = dir.files[i];
        }
    }

    Dir& operator=(const Dir& dir) {
        if(this == &dir){ // 解决自赋值的问题
            return *this;
        }
        this->size = dir.size;
        this->dirPath = dir.dirPath;
        files = new string[size];
        for (int i = 0; i < size; ++i) {
            files[i] = dir.files[i];
        }
        return *this;

    }

    ~Dir(){
        delete[] files;
    }
}Dir;



/**
 * 源代码类型映射
 */
extern std::map<string,int> SourceFileTypeMap;


/**
 * 资源限制结构体
 */
typedef struct ResourceLimit {
    double cpuTime; // 单位为 ms
    double realTime; // ms
    size_t memory; // 单位为 byte
    size_t stack; // byte
    size_t outputSize; // byte

    ResourceLimit(){
        cpuTime = 10; // seconds
        realTime = 20;// seconds
        memory = 5000; // KB
        outputSize = 200; // KB
        stack = 2000; // KB
    }

    string toString() const{
        return string("{ cpu time:") +  std::to_string(cpuTime*1000) + string(" ms\n") +
               string("  real time:") + std::to_string(realTime*1000) + string(" ms\n") +
               string("  memory:") + std::to_string(memory) + string(" KB\n") +
               string("  outputSize:") + std::to_string(outputSize) + string(" KB\n") +
               string("  stack:") + std::to_string(stack) + string(" KB }\n");
    }

}ResourceLimit;



//************************全局函数申明*************************************
/**
 * 获取当前字符串形式的时间
 * @return (example:1970-01-01-12:00:00)
 */
string getCurrentFormattedTime();

/**
 * 执行shell命令
 * @param cmd 要执行的shell命令
 * @param output 存放命令输出信息的变量
 * @return 0表示执行成功，否则返回errno的值
 */
int  execShellCommand(const string& cmd,string& output);

/**
 * 执行shell命令
 * @param cmd 不解释
 * @param output 存放命令输出信息的变量
 * @return RV表示执行成功，否则返回errno的值
 */
int  execShellCommandPlus(const string& cmd,void* args);


/**
 * 获取文件的扩展名
 * @param fileName 文件名
 * @return
 */
SourceFileType getExternalName(const string& fileName);

/**
 * 获取Linux当前用户的家目录
 * @return 家目录
 */
string getHomeDirectory();

/**
  * 获取目录中的所有文件
  * @param dirPath
  * @return
  */
Dir getFilesOfDir(string dirPath);

/**
 * 获取目录中的所有文件，文件名为全路径
 * @param dirPath
 * @return
 */
Dir getFilesOfDirWithFullPath(string dirPath);

/**
 *  从文件描述符中读取文件到str，请确保fd有数据，否则会阻塞
 * @param fd
 * @param str
 */
void readFromFD(int fd,string& str);

/**
 * 判断是否是root用户
 * @return
 */
bool isRoot();

/**
 * 分割字符串
 * @param str
 * @param strs
 * @param delimiters
 */
void split(const string& str,vector<string>& strs,const string& delimiters = " ");


//****************************全局函数申明完毕*******************************


//***************************私有静态函数申明*******************************

//**************************私有静态函数申明完毕*****************************


#endif //JUDGERTOTEACHING_UTIL_H