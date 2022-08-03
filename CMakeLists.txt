# Copyright (c) 2021-present, Zejun Wang (wangzejunscut@126.com).
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -fPIC")

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

add_executable(demo demo.cc)
target_link_libraries(demo tokenizer_static_lib)
