cmake_minimum_required(VERSION 3.27)

project(parser)
set(SOURCE_LIB Parser.cpp)
add_library(parser STATIC ${SOURCE_LIB})