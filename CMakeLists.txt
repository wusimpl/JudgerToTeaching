cmake_minimum_required(VERSION 3.10)
project(JudgerToTeaching)
set(CMAKE_CXX_STANDARD 11)
set(linuxUser none)
set(temp none)
set(linuxUserCMD whoami)
set(userHomeDir none)
message("************************************************************************************************************************************************************")
#获取linux当前用户家目录
execute_process(COMMAND ${linuxUserCMD} OUTPUT_VARIABLE temp)
string(REGEX REPLACE "\n$" "" linuxUser "${temp}") #去除换行
message("current user:" ${linuxUser})
if("${linuxUser}" STREQUAL "root")
    set(userHomeDir /root)
else()
    set(userHomeDir /home/ "${linuxUser}")
endif()
message("user home dir:" "${userHomeDir}")
# 拼接程序工作目录
string(APPEND configDir  ${userHomeDir} /.ets/judge)
message("configDir:" ${configDir})
set(cleanDir ${configDir} "/JudgeExeFilePath/* " ${configDir} "/JudgeOutputFilePath/*")
message("cleanDir:" ${cleanDir})
#创建程序工作目录
set(mkdirCMD "mkdir " ${configDir})
MESSAGE("mkdirCMD:" ${mkdirCMD})
set(mkdirResult success)
execute_process(COMMAND mkdir -p ${configDir})
file(MAKE_DIRECTORY ${configDir})
execute_process(COMMAND ${CMAKE_COMMAND} -E
        copy "${CMAKE_CURRENT_SOURCE_DIR}/config.ini"  "${configDir}"
        )
#添加pthread库，seccomp库
link_libraries(seccomp)
link_libraries(pthread)
message("************************************************************************************************************************************************************")
execute_process(COMMAND rm -r ${configDir})
add_executable(JudgerToTeaching main.cpp compiler/Compiler.h utils/util.h controller/ProgramController.cpp controller/ProgramController.h compiler/CCompiler.cpp compiler/CCompiler.h compiler/CXXCompiler.cpp compiler/CXXCompiler.h checker/AnswerChecker.cpp checker/AnswerChecker.h controller/SubProcess.cpp controller/SubProcess.h)
#清理程序运行产生的文件
#add_custom_target(clean-ets-cache-files COMMAND /usr/bin/cmake -E remove_directory "${cleanDir}")
