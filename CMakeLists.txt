cmake_minimum_required(VERSION 3.27)
project(timetable)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_GENERATOR MATCHES "Visual Studio")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fdiagnostics-color=always")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fsanitize=address")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -flto=auto -march=native -mtune=native -DNDEBUG")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /std:c++17")
endif()

include_directories(include)
set(SOURCE_EXE src/Source.cpp)
add_executable(timetable ${SOURCE_EXE})

add_subdirectory(libs/manager)
add_subdirectory(libs/params)
add_subdirectory(libs/parser)

find_package(Boost COMPONENTS date_time REQUIRED)
find_package(CURL REQUIRED)
find_package(PUGIXML REQUIRED)

target_link_libraries(timetable PUBLIC manager params parser PRIVATE boost_date_time curl pugixml)