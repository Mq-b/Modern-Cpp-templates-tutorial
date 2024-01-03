# 指定 cmake 最小版本号
cmake_minimum_required (VERSION 3.8)

# 指定项目名称

project ("class_template")

# 设置exe的输出目录${PROJECT_SOURCE_DIR}就是当前顶级目录，然后创建bin文件夹里生成exe
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 将源代码添加到此项目的可执行文件
add_executable (class_template  "main.cpp"  "test_class_template.cpp" "test_class.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET class_template PROPERTY CXX_STANDARD 20)
endif()