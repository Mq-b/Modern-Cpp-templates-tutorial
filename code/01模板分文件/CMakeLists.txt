project ("template")                                                # 设置项目名称

cmake_minimum_required (VERSION 3.8)                                # 设置 cmake 版本

set(CMAKE_CXX_STANDARD 23)                                          # 设置 C++ 标准

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)               # 设置输出文件的目录

add_executable (template "main.cpp" "test.cpp" "test_template.cpp") # 设置参与编译的翻译单元

