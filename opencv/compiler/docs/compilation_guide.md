# OpenCV Android ARM64 编译指南

## 1. 🔧 环境准备

### 1.1 系统要求

- **操作系统**: Linux (Ubuntu 18.04+ 推荐)
- **内存**: 最小 8GB，推荐 16GB 以上
- **磁盘空间**: 最小 20GB 可用空间
- **网络**: 稳定的互联网连接（首次构建需要下载源码）

### 1.2 软件依赖

```bash
# 必需软件包
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    wget \
    unzip \
    git \
    python3

# Android 开发环境
# Android NDK r21+ (推荐 r26d)
# Android SDK (API level 21+)
```

### 1.3 环境变量配置

```bash
# 添加到 ~/.bashrc 或 ~/.profile
export ANDROID_NDK_HOME=/home/frewen/snap/Android/ndk/android-ndk-r26d
export ANDROID_SDK_ROOT=/home/frewen/Android/Sdk
export ANDROID_HOME=/home/frewen/Android/Sdk

# 验证环境变量
echo "Android NDK: $ANDROID_NDK_HOME"
echo "Android SDK: $ANDROID_SDK_ROOT"
```

## 2. 🚀 构建步骤详解

### 2.1 完整构建流程

```bash
# 步骤 1: 进入项目目录
cd /home/frewen/01.WorkSpace/AuraKaleidos/FantasyCXX/3rdparty/opencv/compiler

# 步骤 2: 设置环境变量
export ANDROID_NDK_HOME=/home/frewen/snap/Android/ndk/android-ndk-r26d
export ANDROID_SDK_ROOT=/home/frewen/Android/Sdk
export ANDROID_HOME=/home/frewen/Android/Sdk

# 步骤 3: 执行构建
./scripts/build-android-armv8a-release.sh
```

### 2.2 构建过程详解

#### 阶段 1: 源码准备
```
[===Compiler===] './src/opencv-4.11.0.zip' File exists. Ignore Download!!!
[===Compiler===] ./src/opencv_contrib-4.11.0.zip File exists. Ignore Download!!!
[===Compiler===] target download success !!!
```

**说明**: 如果源码已存在则跳过下载，否则自动下载 OpenCV 4.11.0 源码和扩展模块。

#### 阶段 2: 源码解压
```
[===Compiler===] ./src/opencv-4.11.0 Dir exists. Ignore unzip!!!
[===Compiler===] ./src/opencv_contrib-4.11.0/modules Dir exists. Ignore unzip!!!
[===Compiler===] target unzip success !!!
```

**说明**: 解压下载的源码包到 src/ 目录。

#### 阶段 3: CMake 配置
```
[===Compiler===] build target:android-arm64-v8a, build type:release, toolchain:/home/frewen/snap/Android/ndk/android-ndk-r26d/build/cmake/android.toolchain.cmake
[===Compiler===] begin build target in (android-arm64-v8a-release)
-- ANDROID_PLATFORM not set. Defaulting to minimum supported version 21.
-- Detected processor: armv7-a
-- The CXX compiler identification is Clang 17.0.2
-- Configuring done
-- Generating done
```

**关键配置项**:
- **目标平台**: android-arm64-v8a
- **编译器**: Clang 17.0.2
- **Android API**: 21
- **工具链**: Android NDK CMake 工具链

#### 阶段 4: 编译过程
```
[===Compiler===] make target with 22 threads
[  0%] Building C object 3rdparty/zlib/CMakeFiles/zlib.dir/adler32.c.o
[ 10%] Building CXX object modules/core/CMakeFiles/opencv_core.dir/src/matrix.cpp.o
...
[100%] Built target opencv_java
[===Compiler===] make target success!!!
```

**编译特点**:
- 使用 22 个并行线程
- 包含 3rdparty 库和 OpenCV 模块
- 生成 Java 绑定

#### 阶段 5: 安装
```
[===Compiler===] install target success!!!
```

**安装内容**:
- 静态库文件 (.a)
- 头文件
- CMake 配置文件
- 测试程序

## 3. 🔧 构建参数详解

### 3.1 平台选择参数

| 参数 | 平台 | 说明 |
|------|------|------|
| `-t 0` | 主机平台 | macOS/Linux x86_64 |
| `-t 1` | android-armv7a | Android ARM32 |
| `-t 2` | android-armv8a | Android ARM64 ⭐ |
| `-t 3` | android-x86 | Android x86 |
| `-t 4` | android-x86_64 | Android x86_64 |

### 3.2 构建类型参数

| 参数 | 说明 |
|------|------|
| `-r` 或 `--release` | Release 模式（优化） |
| `-d` 或 `--debug` | Debug 模式（调试信息） |
| `--RelWithDebInfo` | 带调试信息的 Release |

### 3.3 路径参数

| 参数 | 说明 |
|------|------|
| `-i <path>` | 安装路径 |
| `-n <name>` | 项目名称（默认 opencv） |
| `-v <version>` | 版本号（默认 4.11.0） |

### 3.4 CMake 参数

**cmake_args/common_options.txt 内容:**

```cmake
# 图像格式支持
-DBUILD_JPEG=ON
-DBUILD_PNG=ON
-DBUILD_TIFF=ON
-DBUILD_WEBP=ON
-DBUILD_OPENEXR=ON

# 核心功能
-DBUILD_opencv_core=ON
-DBUILD_opencv_imgproc=ON
-DBUILD_opencv_dnn=ON
-DBUILD_opencv_java=ON

# 硬件加速
-DWITH_CAROTENE=ON
-DWITH_TBB=ON
-DWITH_OPENMP=ON

# 其他功能
-DBUILD_SHARED_LIBS=ON
-DBUILD_EXAMPLES=ON
-DBUILD_TESTS=ON
```

## 4. 📊 构建配置选项

### 4.1 图像处理格式

```cmake
# 启用 JPEG 支持
-DBUILD_JPEG=ON
-DWITH_JPEG=ON

# 启用 PNG 支持
-DBUILD_PNG=ON
-DWITH_PNG=ON

# 启用 TIFF 支持
-DBUILD_TIFF=ON
-DWITH_TIFF=ON

# 启用 WebP 支持
-DBUILD_WEBP=ON
-DWITH_WEBP=ON

# 启用 OpenEXR 支持
-DBUILD_OPENEXR=ON
-DWITH_OPENEXR=ON
```

### 4.2 核心模块

```cmake
# 核心功能模块
-DBUILD_opencv_core=ON

# 图像处理模块
-DBUILD_opencv_imgproc=ON

# 相机标定和 3D 重建
-DBUILD_opencv_calib3d=ON

# 特征检测
-DBUILD_opencv_features2d=ON

# 目标检测
-DBUILD_opencv_objdetect=ON

# 视频处理
-DBUILD_opencv_video=ON
-DBUILD_opencv_videoio=ON

# 机器学习
-DBUILD_opencv_ml=ON

# 计算摄影
-DBUILD_opencv_photo=ON

# 图像拼接
-DBUILD_opencv_stitching=ON

# 深度神经网络
-DBUILD_opencv_dnn=ON

# 图 API
-DBUILD_opencv_gapi=ON
```

### 4.3 平台特定选项

```cmake
# Android 特定配置
-DWITH_ANDROID_MEDIANDK=ON

# ARM NEON 优化
-DWITH_CAROTENE=ON

# 多线程支持
-DWITH_TBB=ON
-DWITH_OPENMP=ON

# Java 绑定
-DBUILD_opencv_java=ON
-DBUILD_SHARED_LIBS=ON

# 测试和示例
-DBUILD_TESTS=ON
-DBUILD_EXAMPLES=ON
```

## 5. 🛠️ 高级配置

### 5.1 自定义构建类型

```bash
# Debug 构建
./build.sh -t 2 -d -n "opencv" -v "4.11.0" -i "./debug_build"

# Release 构建（默认）
./build.sh -t 2 -r -n "opencv" -v "4.11.0" -i "./release_build"

# 带调试信息的 Release
./build.sh -t 2 --RelWithDebInfo -n "opencv" -v "4.11.0"
```

### 5.2 自定义 CMake 参数

```bash
# 添加额外 CMake 参数
./build.sh -t 2 -r -a "-DWITH_CUDA=ON -DCUDA_ARCH_BIN=7.5"

# 禁用特定模块
./build.sh -t 2 -r -a "-DBUILD_opencv_dnn=OFF -DBUILD_opencv_gapi=OFF"
```

### 5.3 并行编译优化

```bash
# 修改并行线程数（在 build.sh 中）
# 找到以下行并修改:
threads=$(( $cpu - 2 ))  # 默认值
threads=$(( $cpu - 4 ))  # 更保守的设置
threads=$cpu             # 最大并行度
```

## 6. 🔍 构建验证

### 6.1 检查构建结果

```bash
# 检查库文件
ls -la build/android-arm64-v8a-release/lib/armeabi-v7a/

# 检查测试程序
ls -la build/android-arm64-v8a-release/bin/

# 检查头文件
ls -la build/android-arm64-v8a-release/include/

# 检查 CMake 配置
ls -la build/android-arm64-v8a-release/*.cmake
```

### 6.2 验证库文件完整性

```bash
# 检查库文件大小
du -sh build/android-arm64-v8a-release/lib/armeabi-v7a/*.a

# 预期输出:
# 29M  libopencv_core.a
# 33M  libopencv_imgproc.a
# 107M libopencv_dnn.a
# ...
```

### 6.3 运行基本测试

```bash
# 进入测试目录
cd build/android-arm64-v8a-release/bin

# 运行核心功能测试
./opencv_test_core --gtest_filter="Core_*"

# 运行图像处理测试
./opencv_test_imgproc --gtest_filter="Imgproc_*"
```

## 7. 🚨 故障排除

### 7.1 常见构建错误

**错误 1: NDK 路径错误**
```
CMake Error: Could not find toolchain file
```

**解决方案:**
```bash
# 检查并设置正确的 NDK 路径
export ANDROID_NDK_HOME=/correct/ndk/path
```

**错误 2: SDK 路径错误**
```
Android SDK: specify path to Android SDK
```

**解决方案:**
```bash
# 设置 Android SDK 路径
export ANDROID_SDK_ROOT=/correct/sdk/path
export ANDROID_HOME=/correct/sdk/path
```

**错误 3: 内存不足**
```
virtual memory exhausted: Cannot allocate memory
```

**解决方案:**
```bash
# 减少并行编译线程数
# 编辑 build.sh 中的 threads 计算
threads=4  # 或更小值
```

### 7.2 构建性能优化

**问题: 构建速度慢**

**解决方案:**
```bash
# 1. 增加并行线程数
threads=$cpu

# 2. 使用 ccache 加速重复构建
sudo apt-get install ccache
export CCACHE_DIR=/path/to/ccache

# 3. 禁用不需要的模块
./build.sh -t 2 -r -a "-DBUILD_opencv_dnn=OFF"
```

**问题: 磁盘空间不足**

**解决方案:**
```bash
# 1. 清理不需要的构建文件
rm -rf build/android-arm64-v8a-release/CMakeFiles

# 2. 只构建必要的模块
./build.sh -t 2 -r -a "-DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF"

# 3. 使用更小的构建类型
./build.sh -t 2 -r -a "-DCMAKE_BUILD_TYPE=MinSizeRel"
```

## 8. 📈 构建统计

### 8.1 典型构建时间

| 构建类型 | 时间 (分钟) | 说明 |
|----------|-------------|------|
| 首次构建 | 25-35 | 包含源码下载 |
| 增量构建 | 15-25 | 仅编译更改部分 |
| 清理构建 | 20-30 | make clean 后重新构建 |

### 8.2 磁盘空间使用

| 构建阶段 | 空间使用 | 说明 |
|----------|----------|------|
| 源码下载 | 2-3 GB | OpenCV 源码和扩展 |
| 构建过程 | 10-15 GB | 临时文件和对象文件 |
| 最终产物 | 1-2 GB | 库文件和头文件 |
| **总计** | **15-20 GB** | **峰值使用量** |

---

**编译指南结束。如需更多帮助，请参考测试指南和示例代码文档。** 📚