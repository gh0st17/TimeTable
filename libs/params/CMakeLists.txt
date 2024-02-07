cmake_minimum_required(VERSION 3.27)

project(params)
set(SOURCE_LIB Params.cpp)
add_library(params STATIC ${SOURCE_LIB})