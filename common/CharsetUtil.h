//
// Created by root on 2021/4/28.
//

#ifndef JUDGERTOTEACHING_CHARSETUTIL_H
#define JUDGERTOTEACHING_CHARSETUTIL_H

#include "../common/Macro.h"
#define RV_END (-2)

typedef unsigned char byte;
#define EndOfFile(filePtr) (feof(filePtr) == 0 ? false : true)

//n字节序列特征(n:1~4)
extern byte UTF8OctetSequence[10];

//将需要比较的n字节序列的字节可变位置0的掩码
extern byte UTF8OctetSequenceBitClear[10];

#define B1Sequence (&UTF8OctetSequence[0])
#define B2Sequence (&UTF8OctetSequence[1])
#define B3Sequence (&UTF8OctetSequence[3])
#define B4Sequence (&UTF8OctetSequence[5])

#define B1UTF8OctetSequenceBitClear (&UTF8OctetSequenceBitClear[0])
#define B2UTF8OctetSequenceBitClear (&UTF8OctetSequenceBitClear[1])
#define B3UTF8OctetSequenceBitClear (&UTF8OctetSequenceBitClear[3])
#define B4UTF8OctetSequenceBitClear (&UTF8OctetSequenceBitClear[5])

typedef union UTF8Unit{
    byte b1; // 一字节编码(ansi)变量
    byte b2[2]; // 二字节编码变量
    byte b3[3]; // 三字节编码变量
    byte b4[4]; // 四字节编码变量
}UTF8Unit;

typedef enum UTF8UnitType{
    B1, // 一字节类型
    B2,
    B3,
    B4,
    None, // 未知类型，可能不是UTF-8编码
}UTF8UnitType;

#define byteEqual(b1,b2) ((b1)==(b2)?true:false)
typedef struct UTF8UnitWT{ // with type
    UTF8Unit unit;
    UTF8UnitType type;

    /**
     * 两个utf8单元的内容比较
     * @param unit1
     * @param unit2
     * @return 相同返回０，不相同返回１
     */
    bool unitEqual(const UTF8UnitWT* unit1,const UTF8UnitWT* unit2){
        if(unit1->type == unit2->type){
            switch (unit1->type) {
                case B1:
                    return byteEqual(unit1->unit.b1,unit2->unit.b1);
                case B2:
                    return byteEqual(unit1->unit.b1,unit2->unit.b1)
                        && byteEqual(unit1->unit.b2[1],unit2->unit.b2[1]);
                case B3:
                    return byteEqual(unit1->unit.b1,unit2->unit.b1)
                        && byteEqual(unit1->unit.b3[1],unit2->unit.b3[1])
                        && byteEqual(unit1->unit.b3[2],unit2->unit.b3[2]);
                case B4:
                    return byteEqual(unit1->unit.b1,unit2->unit.b1)
                           && byteEqual(unit1->unit.b4[1],unit2->unit.b4[1])
                           && byteEqual(unit1->unit.b4[2],unit2->unit.b4[2])
                           && byteEqual(unit1->unit.b4[3],unit2->unit.b4[3]);
                case None:
                    return true;
            }
        }
        return false;
    }

    bool operator==(const UTF8UnitWT* u){
        return unitEqual(this,u);
    }
}UTF8UnitWT;


/**
 *
 * @param unit
 * @return
 */
static UTF8UnitType getUTF8UnitType(const UTF8Unit& unit){
    if((unit.b1 & B1UTF8OctetSequenceBitClear[0]) == B1Sequence[0]){
        return B1;
    }
    if((unit.b2[0] & B2UTF8OctetSequenceBitClear[0]) == B2Sequence[0]){
       return B2;
    }
    if((unit.b3[0] & B3UTF8OctetSequenceBitClear[0]) == B3Sequence[0]){
        return B3;
    }
    if((unit.b4[0] & B4UTF8OctetSequenceBitClear[0]) == B4Sequence[0]){
        return B4;
    }
    return None;
}

/**
 * 得到下一个utf8单元
 * @param file
 * @param unit 用来存放下一个utf8 unit的联合体
 * @return -2:文件已到达尾部 -1:读取错误 0:成功读取
 */
int getNextUTF8UnitWT(FILE* file,UTF8UnitWT* unitWT);

/**
 * 获取一行数据，就这么简单
 * @param file
 * @param units
 * @return 0表示获取成功,-1表示错误
 */
int getLine(FILE* file,vector<UTF8UnitWT*>* units);

#define NOT_FOUND (-1)
/**
    * 从units的第startPos的位置寻找unit
    * @param units
    * @param startPos 从0开始，懂我意思吧
    * @param unit
    * @return 找不到则返回-1,否则返回unit所在位置
    */
size_t findUTF8UnitWT(const vector<UTF8UnitWT*>* units, size_t startPos, const UTF8UnitWT* unit);



#endif //JUDGERTOTEACHING_CHARSETUTIL_H