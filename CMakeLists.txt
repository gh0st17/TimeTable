cmake_minimum_required(VERSION 3.27)
project(timetable)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fdiagnostics-color=always")

find_package(Boost COMPONENTS date_time REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(CURL REQUIRED)
if(CURL_FOUND)
  include_directories(${CURL_INCLUDE_DIR})
  find_library(CURL_LIBRARY NAMES curl_imp)
endif()

find_package(PugiXML REQUIRED)
if(PugiXML_FOUND)
  find_library(PugiXML_LIBRARY NAMES pugixml)
  find_path(PugiXML_INCLUDE_DIR NAMES pugixml.hpp)
  include_directories(${PugiXML_INCLUDE_DIR})
endif()

include_directories(include)
set(SOURCE_EXE src/Source.cpp)
add_executable(timetable ${SOURCE_EXE})

target_link_libraries(timetable manager params parser ${Boost_LIBRARIES} ${CURL_LIBRARIES} ${PugiXML_LIBRARY})

add_subdirectory(libs/manager)
add_subdirectory(libs/params)
add_subdirectory(libs/parser)