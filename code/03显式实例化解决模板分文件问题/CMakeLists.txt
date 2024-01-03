# 指定 cmake 最小版本号
cmake_minimum_required (VERSION 3.8)

# 指定项目名称

project ("explicit_instantiation")

# 设置exe的输出目录${PROJECT_SOURCE_DIR}就是当前顶级目录，然后创建bin文件夹里生成exe
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 将源代码添加到此项目的可执行文件
add_executable (explicit_instantiation  "main.cpp" "test_function_template.cpp" "test_class_template.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET explicit_instantiation PROPERTY CXX_STANDARD 20)
endif()