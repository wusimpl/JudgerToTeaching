//
// Created by WilliamsAndy on 2021/4/7.
//

#ifndef JUDGERTOTEACHING_ANSWERCHECKER_H
#define JUDGERTOTEACHING_ANSWERCHECKER_H

#include "../common/Macro.h"
#include "../common/Util.h"
#include "../common/CharsetUtil.h"

#define Min(a,b) ((a)>=(b)?(b):(a))

#define RV_WRONG 1

typedef struct CompareDetail{
    long matchingCharNum; // 与标准文本相匹配的字符数
    long lostCharNum; // 丢失的字符数
    long redundantCharNum; // 多余的字符数

    CompareDetail(){
        matchingCharNum = lostCharNum = redundantCharNum = 0;
    }
}CompareDetail;

typedef struct CompareResult{

    /**
     * For byte:
     *  RV_OK:字节流相同 RV_WRONG:字节流不同 RV_ERROR:对比出现错误
     * For char:
     * RV_OK:无意义 RV_WRONG:无意义 RV_ERROR：对比出现错误
     */
    int statusCode; //RV_OK RV_WRONG RV_ERROR

    double lineNumber; // 单个测试文件的行数

    string testFilePath;

    vector<CompareDetail*> linesDetail; // 每一行的对比数据详情


    CompareResult(){
        statusCode = RV_OK;
        lineNumber = 0;
    }

    ~CompareResult(){
        for (auto & i : linesDetail) {
            delete i;
        }
        linesDetail.clear();
    }

}CompareResult;



#define BYTE_COMPARE_METHOD 1
#define CHAR_COMPARE_METHOD 2
/**
 * 答案检查器
 */
class AnswerChecker {
private:
    Dir textGroup1; // 第一组文本数据所在文件夹
    Dir textGroup2; // 第二组文本数据
//    size_t size; // 每组文本的个数，要求两组数量相同 (Deprecated, this info has been included in the Dir object.)
    int compareMethod; // 1.byte 2.char

public:
    CompareResult* compare();

public:
    /**
     * 注意：只接受文件夹路径，要单文件比较，请使用静态函数xxx
     * @param path1
     * @param path2
     */
    explicit AnswerChecker(string path1,string path2);

    /**
     * 返回每组文本的个数
     * @return 如果两组文本数量相同，则返回真实数量，否则返回min(size1,size2)
     */
    int getSize() const;

private:
    /**
     * 严格对比模式：按字节依次对比两个文本
     * @param standardTextPath
     * @param checkTextPath
     * @return
     */
//     int compareByByte(const char* standardTextPath,const char* checkTextPath);


    /**
     * 字符对比模式，逐行进行对比，返回各种类型的字符统计数量，方便调用者进一步给出相似度。
     * @param standardTextPath
     * @param checkTextPath
     * @return
     */
    CompareResult* compareByTextByLine(const char* standardTextPath,const char* checkTextPath,CompareResult* compareResult);

private:
    //返回值不用后不需要自己delete
    CompareResult* compareByTextByLine();
    //返回值不用后不需要自己delete
//    CompareResult* compareByByte();
};


#endif //JUDGERTOTEACHING_ANSWERCHECKER_H
