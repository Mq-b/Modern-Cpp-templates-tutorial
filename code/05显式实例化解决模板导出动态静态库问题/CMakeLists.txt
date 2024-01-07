cmake_minimum_required (VERSION 3.8)

project ("template_explicit_instantiation_dll_lib")


# 设置exe的输出目录${PROJECT_SOURCE_DIR}就是当前顶级目录，然后创建bin文件夹里生成exe
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable (template_explicit_instantiation_dll_lib "main.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET template_explicit_instantiation_dll_lib PROPERTY CXX_STANDARD 20)
endif()