cmake_minimum_required(VERSION 3.17)
project(JudgerToTeaching)

set(CMAKE_CXX_STANDARD 11)

add_executable(JudgerToTeaching main.cpp Compiler/Compiler.cpp Compiler/Compiler.h utils/Data.h utils/util.h utils/debug.h)