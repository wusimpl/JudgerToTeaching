cmake_minimum_required(VERSION 3.10)
project(JudgerToTeaching)
set(CMAKE_CXX_STANDARD 11)
set(LinuxUserName none)

message("************************************************************************************************************************************************************")
#获取linux用户名
execute_process(COMMAND whoami OUTPUT_VARIABLE LinuxUserName)
string(REPLACE "\n" "" LinuxUserName ${LinuxUserName}) #去除换行
message("detected current user:" ${LinuxUserName})
string(APPEND configDir /home/ ${LinuxUserName} /.ets/judge) #拼接家目录

#创建程序工作目录
file(MAKE_DIRECTORY ${configDir})
file(COPY config.ini DESTINATION ${configDir})
message("uploaded config.ini")

#删除编译产生的文件
set(JudgeExeFilePath ${configDir})
set(JudgeOutputFilePath ${configDir})
set(deleteCmd "rm -rf ")
string(APPEND JudgeExeFilePath /JudgeExeFilePath/*)
string(APPEND JudgeOutputFilePath /JudgeOutputFilePath/*)
string(APPEND deleteCmd ${JudgeExeFilePath} " " ${JudgeOutputFilePath})
message(${deleteCmd})
execute_process(COMMAND ${deleteCmd})

#添加pthread库
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
message("************************************************************************************************************************************************************")
add_executable(JudgerToTeaching main.cpp compiler/Compiler.h utils/util.h controller/ProgramController.cpp controller/ProgramController.h compiler/CCompiler.cpp compiler/CCompiler.h compiler/CXXCompiler.cpp compiler/CXXCompiler.h checker/AnswerChecker.cpp checker/AnswerChecker.h)