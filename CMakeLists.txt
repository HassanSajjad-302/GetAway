cmake_minimum_required(VERSION 3.5)

project(GetAway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set( CMAKE_VERBOSE_MAKEFILE CACHE BOOL FORCE)
#cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON


option(USE_CLANG "build application with clang" ON) # OFF is the default
option(USE_GCC "build application with gcc" OFF) # OFF is the default

if(USE_GCC)
    set(CMAKE_C_COMPILER "/usr/bin/gcc")
    set(CMAKE_CXX_COMPILER "/usr/bin/g++")
endif()

if(USE_CLANG)
    set(CMAKE_C_COMPILER "/usr/bin/clang-10")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++-10")
endif()

file(GLOB src_files "${CMAKE_CURRENT_SOURCE_DIR}/Source/src/*.cpp" )
message("src_files value is ${src_files}")
add_executable(GetAway "${CMAKE_CURRENT_SOURCE_DIR}/Source/main.cpp" ${src_files})
target_include_directories(GetAway PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source/header/")
if (USE_GCC)
target_compile_options(GetAway PRIVATE -fcoroutines)
endif()

set(Boost_NO_SYSTEM_PATHS TRUE)
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "/home/hassan/Music/Boost_Installed")
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)
find_package(Boost 1.74.0 REQUIRED)

message(${Boost_INCLUDE_DIRS})
target_include_directories(GetAway PUBLIC ${Boost_INCLUDE_DIRS})


    find_package(Threads REQUIRED)
    target_link_libraries(GetAway Threads::Threads)


