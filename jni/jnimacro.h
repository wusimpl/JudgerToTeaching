//
// Created by root on 2021/5/8.
//

#ifndef JUDGERTOTEACHING_JNIMACRO_H
#define JUDGERTOTEACHING_JNIMACRO_H

//Java基本类型与String类型签名
#define STRING_FIELD_SIG "Ljava/lang/String;"
#define INT_FIELD_SIG "I"
#define BOOLEAN_FIELD_SIG "Z"
#define BYTE_FIELD_SIG "B"
#define CHAR_FIELD_SIG "C"
#define SHORT_FIELD_SIG "S"
#define LONG_FIELD_SIG "J"
#define FLOAT_FIELD_SIG "F"
#define DOUBLE_FIELD_SIG "D"

#define FIND_JAVA_CLASS(classQualifiedName) (env->FindClass(classQualifiedName))
#define GET_JAVA_OBJECT(obj) (env->GetObjectClass(obj))

/**
 * 判空
 */
#define JNI_NOT_NULL(var) if((var)==nullptr){DEBUG_PRINT("!!!!!!!!!!null detected!!!!!!!!!");}

/**
 * type参数可以是 [Object,Boolean,Int,Char,Double,Float,Long,Byte]
 * 请永远在调用包含此宏的宏之前将jni interface 变量名定义为env
 */
#define JNI_INSTANCE_FIELD_VALUE(ins,fieldSig,fieldName,type) \
( \
    ((env)->Get##type##Field( ins,(env)->GetFieldID(GET_JAVA_OBJECT(ins),fieldName,fieldSig) )) \
)

#define JNI_INSTANCE_SET_FIELD_VALUE(ins,fieldSig,fieldName,type,value) \
( \
    ((env)->Set##type##Field( ins,(env)->GetFieldID(GET_JAVA_OBJECT(ins),fieldName,fieldSig),value )) \
)


// string
#define JNI_INSTANCE_STRING_FIELD_VALUE(ins,fieldName) \
    ((jstring)JNI_INSTANCE_FIELD_VALUE(ins,STRING_FIELD_SIG,fieldName,Object))
#define JNI_INSTANCE_SET_STRING_FIELD_VALUE(ins,fieldName,value) \
    (JNI_INSTANCE_SET_FIELD_VALUE(ins,STRING_FIELD_SIG,fieldName,Object,value))

// java unicode str to c utf-8 str
#define JstrToCstr(jstr) ( env->GetStringUTFChars(jstr,NULL) )

//create jstr. 使用后要删除引用
#define CreateJstr(cstr) ((env)->NewStringUTF(cstr))
#define CreateJstrFromCstr(cstr) ( CreateJstr((cstr).c_str()) )

//删除局部引用
#define DeleteLocalRef(ref) (env->DeleteLocalRef(ref))

// 要用到的Java类的全限定名
#define COMPILE_RESULT_CLASS_QUALIFIED_NAME "com/cqjtu/ets/onlinejudge/jni/result/CompileResult"
#define CONTROLLER_RESULT_CLASS_QUALIFIED_NAME "com/cqjtu/ets/onlinejudge/jni/result/ControllerResult"
#define COMPARE_RESULT_CLASS_QUALIFIED_NAME "com/cqjtu/ets/onlinejudge/jni/result/CompareResult"
#define COMPARE_DETAIL_CLASS_QUALIFIED_NAME "com/cqjtu/ets/onlinejudge/jni/result/CompareDetail"

#define RESOURCE_LIMIT_SIG "Lcom/cqjtu/ets/onlinejudge/jni/datastructure/ResourceLimit;"




// 只能用于基本数据类型
template<class T>
T getFieldValue(JNIEnv* env,jobject obj,const char* fieldName,const char* fieldSig){
    jclass cls = env->GetObjectClass(obj);
    if(cls == nullptr){
        return -1;
    }
    jfieldID jfieldId = env->GetFieldID(cls,fieldName,fieldSig);
    T var;

    if(strcmp(fieldSig,INT_FIELD_SIG) == 0){
        var=env->GetIntField(obj,jfieldId);
    }else if(strcmp(fieldSig,SHORT_FIELD_SIG) == 0){
        var=env->GetShortField(obj,jfieldId);
    }else if(strcmp(fieldSig,LONG_FIELD_SIG) == 0){
        var=env->GetLongField(obj,jfieldId);
    }else if(strcmp(fieldSig,FLOAT_FIELD_SIG) == 0){
        var=env->GetFloatField(obj,jfieldId);
    }else if(strcmp(fieldSig,BOOLEAN_FIELD_SIG) == 0){
        var=env->GetBooleanField(obj,jfieldId);
    }else if(strcmp(fieldSig,BYTE_FIELD_SIG) == 0){
        var=env->GetByteField(obj,jfieldId);
    }else if(strcmp(fieldSig,DOUBLE_FIELD_SIG) == 0){
        var=env->GetDoubleField(obj,jfieldId);
    }else{
        DEBUG_PRINT("error! no such field sig!");
        exit(1);
    }

    DeleteLocalRef(cls);

    return var;
}


// 只能用于基本数据类型
template<class T>
bool setFieldValue(JNIEnv* env,jobject obj,const char* fieldName,const char* fieldSig,T value){
    jclass cls = env->GetObjectClass(obj);
    if(cls == nullptr){
        return false;
    }
    jfieldID jfieldId = env->GetFieldID(cls,fieldName,fieldSig);
    if(jfieldId == nullptr){
        return false;
    }

    if(strcmp(fieldSig,INT_FIELD_SIG) == 0){
        env->SetIntField(obj,jfieldId,value);
    }else if(strcmp(fieldSig,SHORT_FIELD_SIG) == 0){
        env->SetShortField(obj,jfieldId,value);
    }else if(strcmp(fieldSig,LONG_FIELD_SIG) == 0){
        env->SetLongField(obj,jfieldId,value);
    }else if(strcmp(fieldSig,FLOAT_FIELD_SIG) == 0){
        env->SetFloatField(obj,jfieldId,value);
    }else if(strcmp(fieldSig,BOOLEAN_FIELD_SIG) == 0){
        env->SetBooleanField(obj,jfieldId,value);
    }else if(strcmp(fieldSig,BYTE_FIELD_SIG) == 0){
        env->SetByteField(obj,jfieldId,value);
    }else if(strcmp(fieldSig,DOUBLE_FIELD_SIG) == 0){
        env->SetDoubleField(obj,jfieldId,value);
    }else{
        DEBUG_PRINT("error! no such field sig!");
        return false;
    }

    DeleteLocalRef(cls);

    return true;
}


// Object类型
jobject getObjectFieldValue(JNIEnv* env,jobject obj,const char* fieldName,const char* fieldSig){
    jclass clz = env->GetObjectClass(obj);
    JNI_NOT_NULL(clz);
    jfieldID jfieldId = env->GetFieldID(clz,fieldName,fieldSig);
    JNI_NOT_NULL(jfieldId);
    jobject var = env->GetObjectField(obj,jfieldId);
    JNI_NOT_NULL(var);

    DeleteLocalRef(clz);

    return var;
}

/**
 * 调用无参构造方法生成Java对象
 * @param env
 * @param className 类名限定符
 * @return 类的实例对象
 */
jobject createJavaObjectUsingZeroParamConstructor(JNIEnv* env, jclass objectClass){
    jmethodID constructorID = env->GetMethodID(objectClass, "<init>", "()V");
    JNI_NOT_NULL(constructorID);
    jobject instance = (env)->NewObject(objectClass,constructorID);
    JNI_NOT_NULL(instance);
    return instance;
}

#endif //JUDGERTOTEACHING_JNIMACRO_H
