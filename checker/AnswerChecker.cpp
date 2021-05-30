//
// Created by WilliamsAndy on 2021/4/7.
//

#include "AnswerChecker.h"
#include <utility>
#define RV_END (-2)

AnswerChecker::AnswerChecker(string stdPath1, string testPath2) {
    textGroup1 = getFilesOfDirWithFullPath(std::move(stdPath1));
    textGroup2 = getFilesOfDirWithFullPath(std::move(testPath2));
    compareMethod=2;
    if(getSize() == -1){
        DEBUG_PRINT("警告：两组文本的数量不相同，这会导致多余的文本被忽略。")
    }
}

int AnswerChecker::getSize() const {
    return textGroup1.size == textGroup2.size ? textGroup1.size : (textGroup1.size>textGroup2.size?textGroup2.size:textGroup1.size);
}

//int AnswerChecker::compareByByte(const char *standardTextPath, const char *checkTextPath) {
//    FILE* stdFile = fopen(standardTextPath,"rb");
//    FILE* checkFile = fopen(checkTextPath,"rb");
//    if(stdFile == nullptr || checkFile == nullptr){
//        DEBUG_PRINT("文件打开失败!");
//        return RV_ERROR;
//    }
//
//    size_t count1,count2;
//    char buf1[1*KB],buf2[1*KB];
//
//    while(!EndOfFile(stdFile) && !EndOfFile(checkFile))
//    {
//        count1 = fread(buf1,sizeof(char),1*KB,stdFile);
//        count2 = fread(buf2,sizeof(char),1*KB,checkFile);
//
//        if(count1 == count2)
//        {
//            for (int i = 0; i < count1; ++i) {
//                if(buf1[i] != buf2[i]){
//                    fclose(stdFile);
//                    fclose(checkFile);
//                    return RV_WRONG;
//                }
//            }
//        }else {
//            fclose(stdFile);
//            fclose(checkFile);
//            return RV_WRONG;
//        }
//    }
//    fclose(stdFile);
//    fclose(checkFile);
//    return RV_OK;
//}


CompareResult* AnswerChecker::compareByTextByLine(const char *standardTextPath, const char *checkTextPath,CompareResult* compareResult) {
    FILE* stdFile = fopen(standardTextPath,"rb");
    FILE* checkFile = fopen(checkTextPath,"rb");
    if(stdFile == nullptr || checkFile == nullptr){
        DEBUG_PRINT("文件打开失败!");
        compareResult->statusCode = RV_ERROR;
        return compareResult;
    }

    int rv1,rv2;
    size_t len1=0,len2=0;
    size_t p1,p2;
    compareResult->lineNumber = 0;

    while(!EndOfFile(stdFile) || !EndOfFile(checkFile)){
        vector<UTF8UnitWT*> units1;
        vector<UTF8UnitWT*> units2;
        rv1 = getLine(stdFile,&units1);
        rv2 = getLine(checkFile,&units2);

        if(rv1 == RV_ERROR || rv2 == RV_ERROR){
            DEBUG_PRINT("读取文件错误!");
            compareResult->statusCode = RV_ERROR;
            fclose(stdFile);
            fclose(checkFile);
            return compareResult;
        }else if(rv1 == RV_END || rv2 == RV_END){ //到达文件尾
            break;
        }

        auto* detail = new CompareDetail();
        if(detail == nullptr){
            DEBUG_PRINT("对比时内存不足！");
            compareResult->statusCode = RV_ERROR;
            return compareResult;
        }
        compareResult->linesDetail.push_back(detail);
        p1=0,p2=0;
        len1 = units1.size();
        len2 = units2.size();
        compareResult->lineNumber++;
        /**
         * 对比算法：stdP与testP相同?匹配字:(testP在stdP后续中存在?(stdP到后续是缺失字):(testP是多余字))
         *  text1 = """我们班里喜欢数学的学生大多数是女生。"""
         *  text2 = """我们班里的学生大多数是女生。"""
         */
        while(p1 < len1 && p2 < len2) {
            if (units1[p1]->operator==(units2[p2])) {
                detail->matchingCharNum++;
                p1++;
                p2++;
            } else {
                size_t pos = 0;
                pos = findUTF8UnitWT(&units1, p1 + 1, units2[p2]);
                if (pos == NOT_FOUND) {
                    detail->redundantCharNum++;
                    p2++;
                } else {
                    detail->matchingCharNum++;
                    detail->lostCharNum += pos - p1;
                    p1 = pos + 1;
                    p2++;
                }
            }
        }
        if(len1 < p1){ // 剩余全是丢失字
            detail->lostCharNum+=len1-p1;
        }
        if(len2 < p2){ // 剩余全是多余字
            detail->redundantCharNum+=len2-p2;
        }

    }

    fclose(stdFile);
    fclose(checkFile);
    return compareResult;
}

CompareResult* AnswerChecker::compareByTextByLine() {
    int size = Min(textGroup1.size,textGroup2.size);
    auto* results = new CompareResult[size];
    for (int i = 0; i < size; ++i) {
        results[i].testFilePath = textGroup1.files[i];
        compareByTextByLine(textGroup1.files[i].c_str(),textGroup2.files[i].c_str(),&results[i]);
    }
    return results;
}

//CompareResult *AnswerChecker::compareByByte() {
//    int size = Min(textGroup1.size,textGroup2.size);
//    auto* results = new CompareResult[size];
//    for (int i = 0; i < size; ++i) {
//        results[i].statusCode = compareByByte(textGroup1.files[i].c_str(),textGroup2.files[i].c_str());
//    }
//    return results;
//}

CompareResult* AnswerChecker::compare(){
    if(compareMethod == BYTE_COMPARE_METHOD){
//        return compareByByte();
            return nullptr;
    }else{
        return compareByTextByLine();
    }
}