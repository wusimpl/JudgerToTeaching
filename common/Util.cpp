//
// Created by root on 2021/4/24.
//

#include "Util.h"
#include "../controller/Pipe3.h"
#include "ConfigurationTool.h"


std::map<string,int> SourceFileTypeMap = {
        {"c",SourceFileType::c},
        {"cc",SourceFileType::cpp},
        {"cpp",SourceFileType::cpp},
        {"java",SourceFileType::java},
        {"py",SourceFileType::py}
};


string getCurrentFormattedTime(){
    time_t currentGMTTime = time(nullptr);
    char buffer[100] = {0};
    strftime(buffer,sizeof(buffer),"%Y-%m-%d-%H:%M:%S",localtime(&currentGMTTime));
    return string(buffer);
}

void readFromFD(int fd, string& str){ // read from file descriptor
    char buf[1024];
    int count;

    count = read(fd,buf,sizeof(buf) - 1);
    while (count > 0){
        str.append(buf);
        count = read(fd,buf,sizeof(buf) - 1);
    };
}



int execShellCommand(const string& cmd,string& output){
    FILE* compilePipe = popen(cmd.c_str(),"r");

    if(compilePipe == nullptr){
        return errno;
    }

    char buffer[1024]={0};

    while(fgets(buffer,sizeof(buffer),compilePipe) != nullptr){
        output.append(buffer);
    }

    pclose(compilePipe);//一定要记得关闭pipe，否则后果严重
    return 0;
}


int execShellCommandPlus(const string& cmd,void* args){
    return pipe3(cmd, reinterpret_cast<Pipe3Run>(pipeRunImpl), args);
}

SourceFileType getExternalName(const string& fileName){
    int dotPosition = fileName.find_last_of(".");
    if (dotPosition == string::npos){ //未找到.
        DEBUG_PRINT("无法获取文件扩展名!\n");
        return SourceFileType::none;
    }
    string suffix = fileName.substr(dotPosition+1,fileName.length()-dotPosition);
    return static_cast<SourceFileType>(SourceFileTypeMap[suffix]);
}


Dir getFilesOfDirWithFullPath(string dirPath) {
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


Dir getFilesOfDir(string dirPath) {
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


string getHomeDirectory(){
    string output;
    execShellCommand("cd && pwd",output);
    return ConfigurationTool::trim(output)+"/";
}

bool isRoot(){
    string user;
    execShellCommand("whoami",user);
    if(user == "root\n"){
        return true;
    }
    return false;
}

char* copyStr(char* dest,const char* src){
    char* p = dest;
    while (*src!='\0'){
        *p = *src;
        p++;
        src++;
    }
    *p = '\0';
    return dest;
}

void split(const string &str,vector<string>& tokens, const string &delimiters) {
    string::size_type lastPos = str.find_first_not_of(delimiters,0);
    string::size_type pos = str.find_first_of(delimiters,lastPos);
    while ( string::npos != pos || string::npos != lastPos){
        tokens.push_back(str.substr(lastPos,pos-lastPos));
        lastPos = str.find_first_not_of(delimiters,pos);
        pos = str.find_first_of(delimiters,lastPos);
    }

}


