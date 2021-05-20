#include "compiler/CCompiler.h"
#include "compiler/CXXCompiler.h"
#include "controller/ProgramController.h"
#include "checker/AnswerChecker.h"
#include <seccomp.h>
#include "jni/com_cqjtu_ets_onlinejudge_jni_JniApplication.h"
#include "jni/jnimacro.h"


/**
 * 供外部调用的编译接口
 * @return
 */
JNIEXPORT jobject JNICALL Java_com_cqjtu_ets_onlinejudge_jni_JniApplication_compile
        (JNIEnv* env, jobject THIS, jobject javaJudgeConfig){

    JudgeConfig nativeCfg;

    //javaJudgeConfig赋值给nativeCfg
    jclass javaCfgCls = GET_JAVA_OBJECT(javaJudgeConfig);
    JNI_NOT_NULL(javaCfgCls);

    jstring codePath = JNI_INSTANCE_STRING_FIELD_VALUE(javaJudgeConfig, "codePath");
    JNI_NOT_NULL(codePath);
    nativeCfg.codePath = JstrToCstr(codePath);

    jstring exePath = JNI_INSTANCE_STRING_FIELD_VALUE(javaJudgeConfig,"exePath");
    JNI_NOT_NULL(exePath);
    nativeCfg.exePath = JstrToCstr(exePath);

    nativeCfg.compileMethod = JNI_INSTANCE_FIELD_VALUE(javaJudgeConfig,INT_FIELD_SIG,"compileMethod",Int);

    Compiler* compiler = nullptr;
    switch (nativeCfg.fileType) {
        case none:
            break;
        case c:
            break;
        case cpp:
            compiler = new CXXCompiler(&nativeCfg);
            break;
        case java:
            break;
        case py:
            break;
    }
    CompileResult compileResult = compiler->compile();
    jclass jCompileResultCls = FIND_JAVA_CLASS(COMPILE_RESULT_CLASS_QUALIFIED_NAME);
    JNI_NOT_NULL(jCompileResultCls);
    jobject jCompileResult = createJavaObjectUsingZeroParamConstructor(env,jCompileResultCls);
    JNI_INSTANCE_SET_STRING_FIELD_VALUE(jCompileResult,"compileOutput", CreateJstrFromCstr(compileResult.compileOutput));
    setFieldValue<int>(env,jCompileResult,"status",INT_FIELD_SIG,compileResult.status);
    setFieldValue<int>(env,jCompileResult,"errnoValue",INT_FIELD_SIG,compileResult.errnoValue);

    delete compiler;
    DeleteLocalRef(javaCfgCls);
    DeleteLocalRef(codePath);
//    DeleteLocalRef(testInPath);
//    DeleteLocalRef(testOutPath);
    DeleteLocalRef(exePath);

    return jCompileResult;
}


/**
 * 供外部调用的运行程序接口
 * @return
 */
JNIEXPORT jobject JNICALL Java_com_cqjtu_ets_onlinejudge_jni_JniApplication_run
        (JNIEnv* env, jobject THIS, jobject javaJudgeConfig){
    JudgeConfig nativeCfg;
    jobject jControllerResult = createJavaObjectUsingZeroParamConstructor(env, FIND_JAVA_CLASS(CONTROLLER_RESULT_CLASS_QUALIFIED_NAME));
    JNI_NOT_NULL(jControllerResult);
    jobject usedResourceLimit = getObjectFieldValue(env,jControllerResult,"usedResourceLimit",RESOURCE_LIMIT_SIG);
    JNI_NOT_NULL(usedResourceLimit);
    //javaJudgeConfig赋值给nativeCfg
    jclass javaCfgCls = GET_JAVA_OBJECT(javaJudgeConfig);
    JNI_NOT_NULL(javaCfgCls);

    jstring testInPath = JNI_INSTANCE_STRING_FIELD_VALUE(javaJudgeConfig, "testInPath");
    JNI_NOT_NULL(testInPath);
    nativeCfg.testInPath = JstrToCstr(testInPath);

//    jstring testOutPath = JNI_INSTANCE_STRING_FIELD_VALUE(javaJudgeConfig, "testOutPath");
//    JNI_NOT_NULL(testOutPath);
//    nativeCfg.testOutPath = JstrToCstr(testOutPath);

    jstring exePath = JNI_INSTANCE_STRING_FIELD_VALUE(javaJudgeConfig, "exePath");
    JNI_NOT_NULL(exePath);
    nativeCfg.exePath = JstrToCstr(exePath);

    jstring outputFilePath = JNI_INSTANCE_STRING_FIELD_VALUE(javaJudgeConfig, "outputFilePath");
    JNI_NOT_NULL(outputFilePath);
    nativeCfg.outputFilePath = JstrToCstr(outputFilePath);

    //程序命令行参数
    jobjectArray programArgs = (jobjectArray)env->GetObjectField(javaJudgeConfig,env->GetFieldID(javaCfgCls,"programArgs","[Ljava/lang/String;"));
    if(programArgs != nullptr){
        jstring jstr;
        const char* cstr= nullptr;
        for (int i = 0; i < env->GetArrayLength(programArgs); ++i) {
            jstr = (jstring)(env->GetObjectArrayElement(programArgs, i));
            cstr = JstrToCstr(jstr);
            nativeCfg.programArgs[i] = new char[strlen(cstr)];
            strcpy(nativeCfg.programArgs[i],cstr);
            DeleteLocalRef(jstr);
        }
    }

    //系统调用
    jint sysCallFilterMode = getFieldValue<int>(env,javaJudgeConfig, "sysCallFilterMode",INT_FIELD_SIG);
    nativeCfg.sysCallFilterMode = sysCallFilterMode;
    jintArray sysCallList = (jintArray)env->GetObjectField(javaJudgeConfig,env->GetFieldID(javaCfgCls,"sysCallList","[I"));
    if(sysCallList!= nullptr){
        for (int i = 0; i < env->GetArrayLength(sysCallList); ++i) {
            nativeCfg.sysCallList[i] = *env->GetIntArrayElements(sysCallList, nullptr);
        }
        DeleteLocalRef(sysCallList);
    }

    // requiredResourceLimit
    jobject requiredLimit = JNI_INSTANCE_FIELD_VALUE(javaJudgeConfig,RESOURCE_LIMIT_SIG,"requiredResourceLimit",Object);
    JNI_NOT_NULL(requiredLimit);
    if(requiredLimit != nullptr){
        nativeCfg.requiredResourceLimit.cpuTime = getFieldValue<double>(env,requiredLimit,"cpuTime",DOUBLE_FIELD_SIG);
//        DEBUG_PRINT("cpu time limited:" << nativeCfg.requiredResourceLimit.cpuTime);
        nativeCfg.requiredResourceLimit.realTime = getFieldValue<double>(env,requiredLimit,"realTime",DOUBLE_FIELD_SIG);
        nativeCfg.requiredResourceLimit.memory = getFieldValue<long>(env,requiredLimit,"memory",LONG_FIELD_SIG);
        nativeCfg.requiredResourceLimit.stack = getFieldValue<long>(env,requiredLimit,"stack",LONG_FIELD_SIG);
        nativeCfg.requiredResourceLimit.outputSize = getFieldValue<long>(env,requiredLimit,"outputSize",LONG_FIELD_SIG);
    }

    ProcessController controller(&nativeCfg);
    ControllerResult controllerResult;
    controllerResult = controller.run();

    DEBUG_PRINT("controller result status:" << controllerResult.status);
    setFieldValue<int>(env,jControllerResult,"status",INT_FIELD_SIG,controllerResult.status);
    switch (controllerResult.status) {
        case 0: // OK
            setFieldValue<double>(env,usedResourceLimit,"cpuTime",DOUBLE_FIELD_SIG,controllerResult.usedResourceLimit.cpuTime);
            setFieldValue<double>(env,usedResourceLimit,"realTime",DOUBLE_FIELD_SIG,controllerResult.usedResourceLimit.realTime);
            setFieldValue<long>(env,usedResourceLimit,"memory",LONG_FIELD_SIG,controllerResult.usedResourceLimit.memory);
            setFieldValue<long>(env,usedResourceLimit,"stack",LONG_FIELD_SIG,controllerResult.usedResourceLimit.stack);
            setFieldValue<long>(env,usedResourceLimit,"data",LONG_FIELD_SIG,controllerResult.usedResourceLimit.data);
            setFieldValue<long>(env,usedResourceLimit,"exeSize",LONG_FIELD_SIG,controllerResult.usedResourceLimit.exeSize);
            setFieldValue<long>(env,usedResourceLimit,"outputSize",LONG_FIELD_SIG,controllerResult.usedResourceLimit.outputSize);
            setFieldValue<int>(env,jControllerResult,"returnValue",INT_FIELD_SIG,controllerResult.returnValue);
            break;
        case 1: // SE
            break;
        default:
            break;
    }

    DeleteLocalRef(javaCfgCls);
    DeleteLocalRef(exePath);
    DeleteLocalRef(outputFilePath);
//    DeleteLocalRef(testOutPath);
    DeleteLocalRef(testInPath);
    DeleteLocalRef(requiredLimit);

    return jControllerResult;
}


/**
 * 答案检查
 * @return
 */
JNIEXPORT jobjectArray JNICALL Java_com_cqjtu_ets_onlinejudge_jni_JniApplication_check
        (JNIEnv* env, jobject THIS, jobject javaJudgeConfig){
    JudgeConfig nativeCfg;

    //javaJudgeConfig赋值给nativeCfg
    jclass javaCfgCls = GET_JAVA_OBJECT(javaJudgeConfig);
    JNI_NOT_NULL(javaCfgCls);

    jstring testInPath = JNI_INSTANCE_STRING_FIELD_VALUE(javaJudgeConfig, "testInPath");
    JNI_NOT_NULL(testInPath);
    nativeCfg.testInPath = JstrToCstr(testInPath);

    jstring testOutPath = JNI_INSTANCE_STRING_FIELD_VALUE(javaJudgeConfig, "testOutPath");
    JNI_NOT_NULL(testOutPath);
    nativeCfg.testOutPath = JstrToCstr(testOutPath);

    jstring outputFilePath = JNI_INSTANCE_STRING_FIELD_VALUE(javaJudgeConfig, "outputFilePath");
    JNI_NOT_NULL(outputFilePath);
    nativeCfg.outputFilePath = JstrToCstr(outputFilePath);

    nativeCfg.compareMethod = getFieldValue<int>(env,javaJudgeConfig, "compareMethod",INT_FIELD_SIG);

    jobjectArray compareResultArray = nullptr;
    if(!nativeCfg.testOutPath.empty() && !nativeCfg.testInPath.empty()) {
        AnswerChecker checker(nativeCfg.testOutPath, nativeCfg.outputFilePath);
        CompareResult* results = nullptr;
        results = checker.compare();
        jclass compareResultCls = FIND_JAVA_CLASS(COMPARE_RESULT_CLASS_QUALIFIED_NAME);
//        jfieldID jfieldId = env->GetFieldID(compareResultCls,"<init>", "()V");
        compareResultArray = env->NewObjectArray(checker.getSize(),compareResultCls,nullptr);
        jobject jCompareResult = nullptr;
        jclass compareDetailCls = FIND_JAVA_CLASS(COMPARE_DETAIL_CLASS_QUALIFIED_NAME);
        jmethodID addCompareDetailID = env->GetMethodID(compareResultCls,"addCompareDetail","(Lcom/cqjtu/ets/onlinejudge/jni/result/CompareDetail;)V");
         for (int i = 0; i < checker.getSize(); ++i) {
            jCompareResult = createJavaObjectUsingZeroParamConstructor(env, compareResultCls);
            setFieldValue<int>(env,jCompareResult,"statusCode",INT_FIELD_SIG,results[i].statusCode);

             jstring testFilePath = CreateJstrFromCstr(results[i].testFilePath);
             JNI_INSTANCE_SET_STRING_FIELD_VALUE(jCompareResult,"testFilePath",testFilePath);

            jobject compareDetail = nullptr;
            for (size_t j = 0; j < results->linesDetail.size(); ++j) {
                 compareDetail = createJavaObjectUsingZeroParamConstructor(env,compareDetailCls);

                setFieldValue<long>(env,compareDetail,"lostCharNum",LONG_FIELD_SIG,results[i].linesDetail[j]->lostCharNum);
                setFieldValue<long>(env,compareDetail,"matchingCharNum",LONG_FIELD_SIG,results[i].linesDetail[j]->matchingCharNum);
                setFieldValue<long>(env,compareDetail,"redundantCharNum",LONG_FIELD_SIG,results[i].linesDetail[j]->redundantCharNum);

                env->CallVoidMethod(jCompareResult,addCompareDetailID,compareDetail);

//                DeleteLocalRef(compareDetail);
             }


            env->SetObjectArrayElement(compareResultArray,i,jCompareResult);

//            DeleteLocalRef(jCompareResult);
        }

        DeleteLocalRef(compareResultCls);
    }

    DeleteLocalRef(outputFilePath);
    DeleteLocalRef(testInPath);
    DeleteLocalRef(testOutPath);
//    DeleteLocalRef(compareResultArray);

    return compareResultArray;
}

int main(int argc,char* argv[]) {
    // 权限检查
    if(!isRoot()){
        DEBUG_PRINT("无root权限!");
        return RV_ERROR;
    }
    //初始化配置文件
    string configPath;
    configPath = getHomeDirectory() + ".ets/judge/config.ini";
    JudgeConfig cfg(configPath);
    if(configPath == string("error")){
        return RV_ERROR;
    }
    cfg.codePath = "/root/test/testprintf.cpp";
    cfg.fileType = getExternalName(cfg.codePath);
    cfg.testInPath = "/root/test/in/";
    cfg.testOutPath = "/root/test/out/";
    cfg.sysCallList[0] = SCMP_SYS(open);
    cfg.sysCallList[1] = SCMP_SYS(read);


    //编译
    Compiler* compiler = nullptr;
    switch (cfg.fileType) {
        case none:
            break;
        case c:
            break;
        case cpp:
            compiler = new CXXCompiler(&cfg);
            break;
        case java:
            break;
        case py:
            break;
    }
    CompileResult compileResult = compiler->compile();
    if(compileResult.status == CompileResult::OK){
        delete compiler;
        // 运行
        ProcessController controller(&cfg);
        ControllerResult controllerResult;
        controllerResult = controller.run();
        if(controllerResult.runStatus == ControllerResult::RunStatus::EXITED_NORMALLY){
            //答案对比
            if(!cfg.testOutPath.empty()){

                AnswerChecker checker(cfg.testOutPath,cfg.outputFilePath);
                CompareResult* results;
//                AnswerChecker::compareByByte("/root/test/output/1.out","/root/test/out/1.out");
//                results = checker.compareByTextByLine();
            }
        }

    } else{
        DEBUG_PRINT("编译错误！");
        DEBUG_PRINT(compileResult.compileOutput);
    }
    return 0;
}
