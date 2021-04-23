//
// Created by rory on 2021/3/5.
//

#ifndef JUDGERTOTEACHING_UTIL_H
#define JUDGERTOTEACHING_UTIL_H

#include <string>
#include <fstream>
#include <iostream>
#include <ios>
#include <cstring>
#include <map>
#include <dirent.h>
#include <unistd.h>
using std::string;
using std::fstream;
using std::getline;

//debug 打印调试输出
#define DEBUG_PRINT(x) std::cout<<x<<std::endl;

#define MAX_SYSCALL_NUMBER 100 // 系统调用配置表最大长度
#define BLACK_LIST_MODE 1 // 系统调用黑名单模式
#define WHITE_LIST_MODE 0 // 系统调用白黑名单模式
#define KB (1024)
#define MB (1024*1024)
#define seconds (1000)
#define minutes (seconds*60)
#define UNLIMITED -1 //资源无限制
#define MAX_PROGRAM_ARGS 100 // 用户程序参数最大数量
#define RV_ERROR -1 // 通用错误返回状态码
#define RV_OK 0 // 通用正确返回状态码

// 加载一条配置
#define LOAD_ONE_CONFIG(key,value) \
 string value; \
 if( !configurationTool.getValue(key,value) ){ \
    DEBUG_PRINT("加载单项配置错误!");        \
    configPath = "error";              \
    return;\
 } \

 // 设置资源限制
#define SetDefaultResourceLimit(var,value) \
 this->requiredResourceLimit.var = value; \

#define MAX_TEST_FILE_NUMBER 20 // 测试数据最大数量
#define KILL_PROCESS(pid) (kill(pid,SIGKILL))  // 用于杀死进程的宏
#define KILLED_SUCCESS nullptr
#define KILLED_ERROR_INFO (&errno)

#define TIME_VALUE(timeval) (timeval.tv_sec*1000 + timeval.tv_usec/1000) // ms
#define R_CPU_TIME(rusage) TIME_VALUE(rusage.ru_utime) // ms

/**
 * 限制进程各类资源的使用的宏
 */
typedef struct rlimit rlimit;
#define SetRLimit_X(X,rlimitStruct) setrlimit(RLIMIT_##X,&(rlimitStruct))



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

    Dir(string directory,int s){
        dirPath = directory;
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
        this->size = dir.size;
        this->dirPath = dir.dirPath;
        files = new string[size];
        for (int i = 0; i < size; ++i) {
            files[i] = dir.files[i];
        }
        return *this;

    }

    ~Dir(){
        if(files){
            delete[] files;
        }
    }
}Dir;

string static getCurrentFormattedTime();
int static execShellCommand(const string& cmd,string& output);
SourceFileType static getExternalName(const string& fileName);
string static getHomeDirectory();
//Dir getFilesOfDir(string dirPath);

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
 * 整个判题流程的评判结果信息
 */
typedef struct WholeResult{
    enum ErrorCode{
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

    }errorCode;

    WholeResult(){
        errorCode = NONE;
    }
}WholeResult;

/**
 * 资源限制结构体
 */
typedef struct ResourceLimit{
    unsigned long cpuTime; // 单位为 ms
    unsigned long realTime; // ms
    unsigned long memory; // 单位为 KB
    unsigned long stack; // KB
    unsigned long outputSize; // KB


    ResourceLimit(){
        cpuTime = 30 * seconds;
        realTime = 2 * minutes;
        memory = 100 * KB;
        outputSize = 10 * KB;
        stack = 200 * KB;
    }
}ResourceLimit;

/**
 * 读取配置文件的类
 */
class ConfigurationTool{
private:
    typedef struct pair{
        string key;
        string value;
    }pair;

    int pairCount;//配置对数量
    pair* pairs;

private:
    /**
     * 获取文件行数
     * @param file 要求为已打开的文件对象，此函数不会在返回时关闭它
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

public:
    /**
     * 去掉字符串首尾空格，\r,\n
     * @param str
     * @return 去掉首尾空格后的字符串
     */
    string static trim(string str){
        str.erase(0, str.find_first_not_of(" \t")); // 去掉头部空格
        str.erase(str.find_last_not_of(" \t") + 1); // 去掉尾部空格
        str.erase(str.find_last_not_of("\r") + 1); // 去掉尾部回车
        str.erase(str.find_last_not_of("\n") + 1); // 去掉尾部换行
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
    JudgeConfig(string& configPath){
//        srcPath  = "";

        string currentTime = getCurrentFormattedTime();

        //读取配置文件
        ConfigurationTool configurationTool;
        bool success = configurationTool.load(configPath);
        if(!success){
            DEBUG_PRINT("配置文件加载错误!\n");
            exit(RV_ERROR);
        }
        LOAD_ONE_CONFIG("JudgeOutputFilePath",JudgeOutputFilePath); // 程序输出文件存放路径
        LOAD_ONE_CONFIG("JudgeExeFilePath",JudgeExeFilePath); // 程序存放路径
        LOAD_ONE_CONFIG("DefaultLimitedCPUTime",DefaultLimitedCPUTime);
        LOAD_ONE_CONFIG("SysCallFilterMode",filterMode); //系统调用过滤模式
        SetDefaultResourceLimit(cpuTime, atoi(DefaultLimitedCPUTime.c_str()));
        LOAD_ONE_CONFIG("DefaultLimitedRealTime",DefaultLimitedRealTime);
        SetDefaultResourceLimit(realTime, atoi(DefaultLimitedRealTime.c_str()));
        LOAD_ONE_CONFIG("DefaultLimitedStack",DefaultLimitedStack);
        SetDefaultResourceLimit(stack, atoi(DefaultLimitedStack.c_str()));
        LOAD_ONE_CONFIG("DefaultLimitedOutputSize",DefaultLimitedOutputSize);
        SetDefaultResourceLimit(outputSize, atoi(DefaultLimitedOutputSize.c_str()));
        LOAD_ONE_CONFIG("DefaultLimitedMemory",DefaultLimitedMemory);
        SetDefaultResourceLimit(memory, atoi(DefaultLimitedMemory.c_str()));

        string homeDirectory = getHomeDirectory();
        JudgeOutputFilePath = homeDirectory + JudgeOutputFilePath;
        JudgeExeFilePath = homeDirectory + JudgeExeFilePath;

        //创建临时工作目录
        string mkdir = string("mkdir -p ").append(JudgeExeFilePath).append(" 2>&1");
        string commandOutputInfo;
        if(execShellCommand(mkdir,commandOutputInfo) != 0){
            DEBUG_PRINT(commandOutputInfo);
            return;
        }

        this->outputFilePath = string(currentTime).insert(0,JudgeOutputFilePath).append("-out/");
        mkdir = string("mkdir -p ").append(outputFilePath).append(" 2>&1");
        if(execShellCommand(mkdir,commandOutputInfo) != 0){
            DEBUG_PRINT(commandOutputInfo);
            return;
        }
        this->exePath = currentTime.insert(0,JudgeExeFilePath).append(".executable");

        sysCallFilterMode = WHITE_LIST_MODE;//默认白名单模式
        if(filterMode == "b"){
            sysCallFilterMode = BLACK_LIST_MODE;
        }else if(filterMode == "w"){

        }else{
            DEBUG_PRINT("系统调用配置出错!设置为白名单模式已默认");
        }
        for (int i = 0; i < MAX_SYSCALL_NUMBER; ++i) {
            sysCallList[i] = -1;
        }

        for (int i = 0; i < MAX_PROGRAM_ARGS; ++i) {
            programArgs[i] = nullptr;
        }
    }

    string srcPath; //源代码文件全路径
    string exePath; //可执行文件全路径
    string testInPath; //测试输入文件路径(文件夹)，每组测试数据都要求与输出文件名配对
    string testOutPath; //测试输出文件路径(文件夹)
    char* programArgs[100]; //被测试程序的参数
    /**
     * question1
     *  in
     *      1.in
     *      2.in
     *  out
     *      1.out
     *      2.out
     */
    string outputFilePath; //输出文件路径(重定向被测程序的stdout到outputFilePath)
    SourceFileType fileType; //源文件类型，帮助调用相应编译器

    int sysCallList[MAX_SYSCALL_NUMBER]; //系统调用名单，与filterMode搭配使用
    int sysCallFilterMode; //true：黑名单模式 false：白名单模式

    ResourceLimit requiredResourceLimit; //题目要求的资源限制值
    ResourceLimit usedResourceLimit; //程序实际所用资源

    WholeResult wholeResult; //所有评判结果
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


 /**
  * 获取目录中的所有文件，文件名为全路径
  * @param directory 请参看结构体定义
  * @return 失败或者成功获取
  */
Dir static getFilesOfDirWithFullPath(string dirPath) {
    DIR* dir = nullptr;
    struct dirent* ptr;
    int size = 0;//总文件数

    if((dir=opendir(dirPath.c_str())) == nullptr){
       return Dir(dirPath,0);
    }
     while ((ptr=readdir(dir))){
         if(ptr->d_type == 8){//which means that it's a file rather than a dir,link file or any other things
             size++;
         }
     }
     closedir(dir);
     opendir(dirPath.c_str());
     Dir myFiles(dirPath,size);
     int i = 0;
     while ((ptr=readdir(dir))){
         if(ptr->d_type == 8){//which means that it's a file rather than a dir,link file or any other things
             myFiles.files[i++] = dirPath.append(ptr->d_name);
         }
     }
     closedir(dir);
    return myFiles;
}

/**
  * 获取目录中的所有文件
  * @param directory 请参看结构体定义
  * @return 失败或者成功获取
  */
Dir static getFilesOfDir(string dirPath) {
    DIR* dir = nullptr;
    struct dirent* ptr;
    int size = 0;//总文件数

    if((dir=opendir(dirPath.c_str())) == nullptr){
        return Dir(dirPath,0);
    }
    while ((ptr=readdir(dir))){
        if(ptr->d_type == 8){//which means that it's a file rather than a dir,link file or any other things
            size++;
        }
    }
    closedir(dir);
    opendir(dirPath.c_str());
    Dir myFiles(dirPath,size);
    int i = 0;
    while ((ptr=readdir(dir))){
        if(ptr->d_type == 8){//which means that it's a file rather than a dir,link file or any other things
            myFiles.files[i++] = ptr->d_name;
        }
    }
    closedir(dir);
    return myFiles;
}

/**
 * 获取Linux当前用户的家目录
 * @return 家目录
 */
string static getHomeDirectory(){
    string output;
    execShellCommand("cd && pwd",output);
    return ConfigurationTool::trim(output)+"/";
}

bool static isRoot(){
    string user;
    execShellCommand("whoami",user);
    if(user == "root\n"){
        return true;
    }
    return false;
}
#endif //JUDGERTOTEACHING_UTIL_H