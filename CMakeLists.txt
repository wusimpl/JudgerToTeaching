cmake_minimum_required(VERSION 3.10)
project(JudgerToTeaching)

set(CMAKE_CXX_STANDARD 11)

add_executable(JudgerToTeaching main.cpp compiler/Compiler.h utils/util.h utils/debug.h ProgramController.cpp ProgramController.h compiler/CCompiler.cpp compiler/CCompiler.h)