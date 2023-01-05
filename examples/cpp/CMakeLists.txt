# Copyright (c) 2022-present, Zejun Wang (wangzejunscut@126.com)
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_executable(demo demo.cc)
target_link_libraries(demo tokenizer_static_lib)

add_executable(speed_tests speed_tests.cc)
target_link_libraries(speed_tests tokenizer_static_lib)
