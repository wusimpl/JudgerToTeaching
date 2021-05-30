//
// Created by root on 2021/4/28.
//

#include "CharsetUtil.h"

/** https://tools.ietf.org/html/rfc3629#page-6
 * Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
   --------------------+---------------------------------------------
   0000 0000-0000 007F | 0xxxxxxx
   0000 0080-0000 07FF | 110xxxxx 10xxxxxx
   0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
   0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

 */
byte UTF8OctetSequence[10] = {
        0b00000000,
        0b11000000,0b10000000,
        0b11100000,0b10000000,0b10000000,
        0b11110000,0b10000000,0b10000000,0b1000000
};

/**
 * 事实上每种字节序列都只需要第一字节的规则，就可以判断这是几字节单元
 */
byte UTF8OctetSequenceBitClear[10] = {
        0b10000000,
        0b11100000,0b11000000,
        0b11110000,0b11000000,0b11000000,
        0b11111000,0b11000000,0b11000000,0b11000000
};

int getNextUTF8UnitWT(FILE* file,UTF8UnitWT* unitWT){
    if(EndOfFile(file)){
        return -2; // 无内容,已到达文件尾
    }

    size_t count=0;
    count = fread(&(unitWT->unit.b1),sizeof(byte),1,file);
    if(count == 0){ //很蒙，不知道为什么EndOfFile有时会失效
        return -2;
    }
    unitWT->type = getUTF8UnitType(unitWT->unit);
    switch (unitWT->type) {
        case B1:
            break;
        case B2:
            count = fread(&(unitWT->unit.b2[1]),sizeof(byte),1,file);
            if(count != 1){
                return RV_ERROR;
            }
            break;
        case B3:
            count = fread(&(unitWT->unit.b3[1]),sizeof(byte),2,file);
            if(count != 2){
                return RV_ERROR;
            }
            break;
        case B4:
            count = fread(&(unitWT->unit.b4[1]),sizeof(byte),3,file);
            if(count != 3){
                return RV_ERROR;
            }
            break;
        case None:
            return RV_ERROR;
    }
    return RV_OK;
}


int getLine(FILE* file,vector<UTF8UnitWT*>* units){
    UTF8UnitWT* unitWT = nullptr;
    int count = 0;
    int rv;

    while(true){
        unitWT = new UTF8UnitWT;
        rv = getNextUTF8UnitWT(file,unitWT);

        if (rv == 0){ // 正常
            if (unitWT->unit.b1 == '\n' || unitWT->unit.b1 == '\r') {
                getNextUTF8UnitWT(file,unitWT); // get the next char '\n' rather than the previous '\r'
                units->push_back(unitWT);
                return RV_OK;
            } else { // 非换行符
                units->push_back(unitWT);
                count++;
            }
        } else if(rv == -2){ //　文件尾
            delete unitWT;
            return RV_END;
        }else{ // rv == -1 (error)
            delete unitWT;
            return RV_ERROR;
        }
    }
}


size_t findUTF8UnitWT(const vector<UTF8UnitWT*>* units, size_t startPos, const UTF8UnitWT* unit){
    if(units->size()-1 < startPos){
        return NOT_FOUND;
    }

    for (size_t i = startPos; i < units->size(); ++i) {
        if((*units)[i]->operator==(unit)){
            return i;
        }
    }
    return NOT_FOUND;
}