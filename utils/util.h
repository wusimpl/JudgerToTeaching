//
// Created by rory on 2021/3/5.
//

#ifndef JUDGERTOTEACHING_UTIL_H
#define JUDGERTOTEACHING_UTIL_H

#include <string>
#include <fstream>
#include <ios>
#include <stdio.h>
#include <cstring>
#include <map>
#include "debug.h"
using std::string;
using std::fstream;
using std::getline;

#ifndef CONFIG_PATH
#define CONFIG_PATH "~/.Judge/config.ini"
#endif

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

string static getCurrentFormattedTime();
int static execShellCommand(const string& cmd,string& output);
SourceFileType static getExternalName(const string& fileName);

/**
 * 源代码类型映射
 */
static std::map<string,int> SourceFileTypeMap = {
        {"c",SourceFileType::c},
        {"cc",SourceFileType::cpp},
        {"cpp",SourceFileType::cpp},
        {"java",SourceFileType::java},
        {"py",SourceFileType::py}
};


/**
 * 评判结果
 */
enum JudgeResult{
    NONE=-1, //初始状态
    CE=0, //Compile Error

    TLE, //Time Limited Error
    MLE, //Memory Limited Error
    OLE, //Output Limited Error, 通常意味着短时间内输出过多
    RE, //Runtime Error
    RF, //Restricted Function, 调用了危险的函数

    WA, //Wrong Answer
    PC, //Partially Correct, 通过了部分测试数据
    AC, //Accepted
    PE, //Presentation Error, 输出格式错误(可能是空格、换行、数值精度等未控制好)

    SE //System Error, 评判系统内部错误

};

/**
 * 读取配置文件的类
 */
class ConfigurationTool{
    class pair{
    public:
        string key;
        string value;
    };

    int pairCount;//配置对数量
    struct pair* pairs;

private:
    /**
     * 获取文件行数
     * @param file 要求未已打开的文件对象，函数不会在执行时关闭它
     * @return 文件行数
     */
    int countLines(fstream& file){
        if(file.fail()){
            return -1;
        }
        int count = 0;
        string buffer;
        while (getline(file,buffer)){
            count++;
        }
        return count;
    }

    /**
     * 去掉字符串首尾空格
     * @param str
     * @return
     */
    string trim(string str){
        str.erase(0, str.find_first_not_of(" \t")); // 去掉头部空格
        str.erase(str.find_last_not_of(" \t") + 1); // 去掉尾部空格
        return str;
    }

public:

    /**
     * 通过key获取value
     * @param k 不解释
     * @param v 要存放value的地方
     * @return 是否找到
     */
    bool getValue(const string& k,string& v){
        for(int i=0;i<pairCount;i++){
            if(pairs[i].key == k){
                v = pairs[i].value;
                return true;
            }
        }
        return false;
    }

    /**
     * 加载配置文件
     * @param filePath
     * @return 表明加载成功与否
     */
    bool load(const string& filePath){
        fstream file(filePath,std::ios::in);
        if(!file.is_open()){
            pairs = nullptr;
            return false;
        }

        pairCount = countLines(file);
        pairs = new pair[pairCount];
        if(!pairs){
            return false;
        }

        file.close();
        file.open(filePath,std::ios::in);
        string buffer;
        int currentLine=1;
        int posOfEquation;
        while (getline(file,buffer)){ //分行读取键值对
            posOfEquation = buffer.find_first_of('=');
            pairs[currentLine-1].key = trim(buffer.substr(0,posOfEquation-1));
            pairs[currentLine-1].value = trim(buffer.substr(posOfEquation+1,buffer.size()-posOfEquation));
            currentLine++;
        }

        file.close();
        return true;
    }

    ~ConfigurationTool(){
        if(pairs){
           delete[] pairs;
        }
    }

};

/*
 * 判题配置
 */
class JudgeConfig{
public:
    JudgeConfig(string configPath){
//        srcPath  = "";

        string currentTime = getCurrentFormattedTime();

        //读取配置文件
        ConfigurationTool configurationTool;
        bool success = configurationTool.load(configPath);
        if(!success){
            DEBUG_PRINT("配置文件加载错误!\n");
            return;
        }
        string __outputFilepath;
        string __exeFilePath;
        configurationTool.getValue("JudgeOutputFilePath",__outputFilepath);
        configurationTool.getValue("JudgeExeFilePath",__exeFilePath);

        //创建临时工作目录
        string mkdir = string("mkdir ").append(__outputFilepath).append(" ").append(__exeFilePath).append(" 2>&1");
        string commandOutputInfo;
        if(execShellCommand(mkdir,commandOutputInfo) != 0){
            DEBUG_PRINT(commandOutputInfo);
            return;
        }

        this->outputFilePath = string(currentTime).insert(0,__outputFilepath).append(".out");
        this->exePath = currentTime.insert(0,__exeFilePath).append(".executable");

        sysCallList = nullptr;
        filterMode = true;

        limitedCPUTime = 1500;
        limitedMemory = 5000;

        judgeResult = NONE;
    }

    string srcPath; //源代码文件全路径
    string exePath; //可执行文件全路径
    string outputFilePath; //输出文件路径(重定向被测程序的stdout到outputFilePath)
    SourceFileType fileType; //源文件类型，帮助调用相应编译器

    int* sysCallList; //系统调用名单，与filterMode搭配使用
    bool filterMode; //true：白名单模式 false：黑名单模式

    int limitedCPUTime; // 单位为 ms
    int limitedMemory; // 单位为 KB

    JudgeResult judgeResult;


};


/*
 *
 */
struct JudgeInfo{
    int usedCPUTime; //被测程序cpu使用时间
    int usedMemory;

};



 /**
  * 获取当前字符串形式的时间
  * @return (example:1970-01-01-12:00:00)
  */
string static getCurrentFormattedTime(){
    time_t currentGMTTime = time(nullptr);
    char buffer[100] = {0};
    strftime(buffer,sizeof(buffer),"%Y-%m-%d-%H:%M:%S",localtime(&currentGMTTime));
    return string(buffer);
}

/**
 * 执行shell命令
 * @param cmd 要执行的shell命令
 * @param output 存放命令输出信息的变量
 * @return 0表示执行成功，否则返回errno的值
 */
int static execShellCommand(const string& cmd,string& output){
    FILE* compilePipe = popen(cmd.c_str(),"r");

    if(compilePipe == nullptr){
       return errno;
    }

    char buffer[1024]={0};
//    memset(buffer,0,1024);
    while(fgets(buffer,sizeof(buffer),compilePipe) != nullptr){
        output.append(buffer);
    }

    pclose(compilePipe);//一定要记得关闭pipe，否则后果严重
    return 0;
}

/**
 * 获取文件的扩展名
 * @param fileName 文件名
 * @return
 */
SourceFileType static getExternalName(const string& fileName){
    int dotPosition = fileName.find_last_of(".");
    if (dotPosition == string::npos){ //未找到.
        DEBUG_PRINT("无法获取文件扩展名!\n");
        return SourceFileType::none;
    }
    string suffix = fileName.substr(dotPosition+1,fileName.length()-dotPosition);
    return static_cast<SourceFileType>(SourceFileTypeMap[suffix]);
}

#endif //JUDGERTOTEACHING_UTIL_H