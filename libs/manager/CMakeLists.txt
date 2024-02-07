cmake_minimum_required(VERSION 3.27)

project(manager)
set(SOURCE_LIB Manager.cpp)
add_library(manager STATIC ${SOURCE_LIB})