cmake_minimum_required(VERSION 3.10)
project(JudgerToTeaching)
set(CMAKE_CXX_STANDARD 11)
set(LinuxUserName none)

#获取linux用户名
execute_process(COMMAND whoami OUTPUT_VARIABLE LinuxUserName)
string(REPLACE "\n" "" LinuxUserName ${LinuxUserName}) #去除换行
string(APPEND configDir /home/ ${LinuxUserName} /.ets/judge) #拼接家目录

#创建程序工作目录
file(MAKE_DIRECTORY ${configDir})
file(COPY config.ini DESTINATION ${configDir})

add_executable(JudgerToTeaching main.cpp compiler/Compiler.h utils/util.h utils/debug.h controller/ProgramController.cpp controller/ProgramController.h compiler/CCompiler.cpp compiler/CCompiler.h compiler/CXXCompiler.cpp compiler/CXXCompiler.h)