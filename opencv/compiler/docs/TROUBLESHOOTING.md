# OpenCV Android ARM64 故障排除指南

## 目录

1. [编译问题](#编译问题)
2. [配置问题](#配置问题)
3. [链接问题](#链接问题)
4. [运行时问题](#运行时问题)
5. [性能问题](#性能问题)
6. [环境问题](#环境问题)

## 编译问题

### 1. CMake工具链错误

#### 错误信息
```
CMake Error: Could not find toolchain file:
/Users/frewen/Library/Android/sdk/ndk/26.3.11579264/build/cmake/android.toolchain.cmake
```

#### 原因分析
- Android NDK路径设置错误
- NDK未正确安装
- 环境变量未正确配置

#### 解决方案
```bash
# 1. 检查NDK实际安装路径
find /Users/frewen -name "android.toolchain.cmake" 2>/dev/null

# 2. 设置正确的环境变量
export ANDROID_NDK_HOME="/Users/frewen/Library/AAura/Android/sdk/ndk/26.3.11579264"

# 3. 验证路径
if [ -f "$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" ]; then
    echo "✅ NDK工具链文件存在"
else
    echo "❌ 请检查NDK安装路径"
fi
```

### 2. ABI检测错误

#### 错误信息
```
-- Detected processor: armv7-a
-- Android ABI: armeabi-v7a (期望: arm64-v8a)
```

#### 原因分析
- CMake参数未正确传递
- Android NDK工具链默认使用armeabi-v7a
- 编译脚本参数配置错误

#### 解决方案
```bash
# 1. 明确指定ABI参数
cmake -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 \
      -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
      ...

# 2. 检查CMakeCache.txt
grep -i "abi" CMakeCache.txt

# 3. 清理并重新配置
rm -rf build/android-arm64-v8a-release
mkdir -p build/android-arm64-v8a-release
cd build/android-arm64-v8a-release
```

### 3. Java环境错误

#### 错误信息
```
ERROR: JAVA_HOME is set to an invalid directory
Please set the JAVA_HOME variable in your environment
```

#### 原因分析
- Java环境配置不正确
- Android项目构建需要Java支持
- JAVA_HOME指向了错误的路径

#### 解决方案
```bash
# 方法1: 禁用Java相关组件（推荐）
cmake -DBUILD_JAVA=OFF \
      -DBUILD_ANDROID_PROJECTS=OFF \
      -DBUILD_ANDROID_EXAMPLES=OFF \
      ...

# 方法2: 设置正确的Java路径
export JAVA_HOME="/Library/Java/JavaVirtualMachines/jdk-20.jdk/Contents/Home"

# 方法3: 临时禁用Java检查
export JAVA_HOME=""
```

### 4. 内存不足错误

#### 错误信息
```
g++: fatal error: Killed program cc1plus
compilation terminated.
```

#### 原因分析
- 系统内存不足
- 并行编译线程过多
- 交换空间不足

#### 解决方案
```bash
# 1. 减少并行编译线程
make -j2  # 而不是 make -j6

# 2. 增加交换空间
sudo dd if=/dev/zero of=/swapfile bs=1G count=4
sudo mkswap /swapfile
sudo swapon /swapfile

# 3. 分模块编译
make opencv_core
make opencv_imgproc
make opencv_imgcodecs
# ...
```

## 配置问题

### 1. CMake参数传递失败

#### 错误信息
```
CMake Warning: Manually-specified variables were not used by the project
```

#### 原因分析
- CMake参数格式错误
- 参数被后续配置覆盖
- CMake版本不兼容

#### 解决方案
```bash
# 1. 检查CMake版本
cmake --version

# 2. 使用正确的参数格式
# 错误: -DBUILD_JAVA = OFF
# 正确: -DBUILD_JAVA=OFF

# 3. 验证参数传递
echo "CMAKE_ARGS: $CMAKE_ARGS"
cmake $CMAKE_ARGS ...
```

### 2. 第三方库依赖问题

#### 错误信息
```
Could NOT find Eigen3 (missing: EIGEN3_INCLUDE_DIR)
Could NOT find Protobuf (missing: PROTOBUF_LIBRARY)
```

#### 原因分析
- 第三方库未正确安装
- CMake查找路径配置错误
- 交叉编译环境路径问题

#### 解决方案
```bash
# 1. 禁用外部依赖（推荐用于Android）
cmake -DWITH_EIGEN=OFF \
      -DWITH_PROTOBUF=ON \
      -DBUILD_PROTOBUF=ON \
      ...

# 2. 手动指定路径
cmake -DEIGEN3_INCLUDE_DIR=/path/to/eigen \
      -DPROTOBUF_INCLUDE_DIR=/path/to/protobuf \
      ...
```

## 链接问题

### 1. 库文件缺失

#### 错误信息
```
ld: library not found for -lopencv_core
```

#### 原因分析
- 库文件未正确生成
- 链接路径配置错误
- 库名称不匹配

#### 解决方案
```bash
# 1. 检查库文件是否存在
ls -la lib/arm64-v8a/libopencv_*.a

# 2. 验证库文件架构
file lib/arm64-v8a/libopencv_core.a

# 3. 检查链接路径
export LIBRARY_PATH="/path/to/opencv/lib/arm64-v8a:$LIBRARY_PATH"

# 4. 使用完整路径链接
g++ -o test test.cpp /full/path/to/libopencv_core.a
```

### 2. 符号未定义

#### 错误信息
```
undefined reference to `cv::imread(...