# CMakeLists.txt
cmake_minimum_required(VERSION 3.4.1)

# OpenCV 包查找
find_package(OpenCV REQUIRED)

# 日志库
find_library(log-lib log)

# 原生库
add_library(native-lib SHARED
            src/main/cpp/native-lib.cpp)

# 包含目录
include_directories(${OpenCV_INCLUDE_DIRS})

# 链接库
target_link_libraries(native-lib
                     ${OpenCV_LIBS}
                     ${log-lib}
                     android
                     z)

# 编译选项
target_compile_options(native-lib PRIVATE
                      -Wall
                      -Wextra
                      -fexceptions
                      -frtti)

# C++ 标准
set_property(TARGET native-lib PROPERTY CXX_STANDARD 11)

# 优化选项
target_compile_options(native-lib PRIVATE
                      -O3
                      -march=armv7-a
                      -mfpu=neon
                      -ffast-math
                      -funroll-loops)

# 预处理器定义
target_compile_definitions(native-lib PRIVATE
                          OPENCV_ENABLED=1
                          USE_ANDROID=1)