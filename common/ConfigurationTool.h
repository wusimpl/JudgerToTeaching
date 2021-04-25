//
// Created by root on 2021/4/25.
//

#ifndef JUDGERTOTEACHING_CONFIGURATIONTOOL_H
#define JUDGERTOTEACHING_CONFIGURATIONTOOL_H

#include "Macro.h"

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
    static int countLines(fstream& file);

public:
    /**
     * 去掉字符串首尾空格，\r,\n
     * @param str
     * @return 去掉首尾空格后的字符串
     */
    static string trim(string str);

public:

    /**
     * 通过key获取value
     * @param k 不解释
     * @param v 要存放value的地方
     * @return 是否找到
     */
    bool getValue(const string& k,string& v);

    /**
     * 加载配置文件
     * @param filePath
     * @return 表明加载成功与否
     */
    bool load(const string& filePath);

    ~ConfigurationTool(){
        delete[] pairs;
    }

};


#endif //JUDGERTOTEACHING_CONFIGURATIONTOOL_H
