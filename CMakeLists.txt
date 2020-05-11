cmake_minimum_required(VERSION 3.15)
project(Interpreter)

set(CMAKE_CXX_STANDARD 14)

add_executable(Test main.cpp)
add_executable(Test2 test2.cpp)
add_executable(Interpreter "parser/Interpreter.cpp")