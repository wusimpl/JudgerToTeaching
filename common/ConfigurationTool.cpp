//
// Created by root on 2021/4/25.
//

#include "ConfigurationTool.h"

int ConfigurationTool::countLines(fstream &file) {
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

string  ConfigurationTool::trim(string str) {
    str.erase(0, str.find_first_not_of(" \t")); // 去掉头部空格
    str.erase(str.find_last_not_of(" \t") + 1); // 去掉尾部空格
    str.erase(str.find_last_not_of('\r') + 1); // 去掉尾部回车
    str.erase(str.find_last_not_of('\n') + 1); // 去掉尾部换行
    return str;
}

bool ConfigurationTool::getValue(const string &k, string &v) {
    for(int i=0;i<pairCount;i++){
        if(pairs[i].key == k){
            v = pairs[i].value;
            return true;
        }
    }
    return false;
}

bool ConfigurationTool::load(const string &filePath) {
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
