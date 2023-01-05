# Copyright (c) 2022-present, Zejun Wang (wangzejunscut@126.com)
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0079 NEW)

project(easytokenizer LANGUAGES CXX C VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -fPIC")

option(WITH_OMP     "Compile with OpenMP"  OFF)
if (WITH_OMP)
    find_package(OpenMP REQUIRED)
    add_definitions(-DWITH_OMP)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

message("CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(tokenizer_static_lib 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tokenizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utf8proc.c)
target_link_libraries(tokenizer_static_lib pthread)
set_target_properties(tokenizer_static_lib PROPERTIES OUTPUT_NAME tokenizer)

add_library(tokenizer_shared_lib SHARED 
${CMAKE_CURRENT_SOURCE_DIR}/src/tokenizer.cc
${CMAKE_CURRENT_SOURCE_DIR}/src/utf8proc.c)
target_link_libraries(tokenizer_shared_lib pthread)
set_target_properties(tokenizer_shared_lib PROPERTIES OUTPUT_NAME tokenizer)

add_subdirectory(examples)
