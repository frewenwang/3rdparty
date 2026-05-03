# OpenCV Android ARM64 构建工程完整文档

## 1. 📖 项目概述

这是一个用于构建 OpenCV 4.11.0 Android ARM64 版本的构建工程。该工程提供了完整的自动化构建脚本，支持交叉编译 OpenCV 库及其扩展模块，生成适用于 Android 平台的静态库和 Java 绑定。

## 2. 🏗️ 项目结构

```
compiler/
├── build.sh                              # 主构建脚本
├── scripts/
│   └── build-android-armv8a-release.sh   # Android ARM64 构建脚本
├── cmake_args/
│   └── common_options.txt               # CMake 配置选项
├── src/                                  # OpenCV 源码目录
├── build/                                # 构建输出目录
│   └── android-arm64-v8a-release/        # Android ARM64 构建结果
└── docs/                                 # 文档目录
    ├── OpenCV_Build_Documentation_zh.md  # 本文档
    ├── compilation_guide.md              # 编译指南
    ├── testing_guide.md                  # 测试指南
    └── examples/                         # 示例代码目录
```

## 3. 🔧 构建脚本详解

### 3.1 build.sh 主构建脚本

```bash
#!/usr/bin/env bash
#################################################################################
# OpenCV 跨平台构建系统
# 
# 功能：
# - 支持多平台交叉编译（Android、iOS、Linux、macOS、QNX）
# - 自动检测主机环境
# - CMake 构建系统配置
# - 并行编译优化
#
# 使用方法：
# ./build.sh [选项]
#
# 选项：
#   -t, --target      目标平台 (0-9)
#   -r, --release     Release 模式构建
#   -d, --debug       Debug 模式构建
#   -i, --install     安装路径
#   -a                CMake 额外参数
#   -h, --help        显示帮助
#################################################################################
```

**核心功能模块：**

1. **环境检测模块**
   ```bash
   # 检测主机操作系统
   if [ "$(uname)" == "Darwin" ]; then
       HOST_OS="mac"
       HOST_ARCH="x86_64"
   elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
       HOST_OS="linux"
       HOST_ARCH="x86_64"
   fi
   ```

2. **参数解析模块**
   ```bash
   # 支持多种构建选项
   while [ $# != 0 ]; do
     case "$1" in
       -t) TARGET_INDEX=$2; shift ;;           # 目标平台
       -r) BUILD_TYPE="release" ;;             # Release 模式
       -d) BUILD_TYPE="debug" ;;               # Debug 模式
       -i) INSTALL_PATH=$2; shift ;;           # 安装路径
       -a) BUILD_CMAKE_ARGS=$2; shift ;;       # CMake 参数
     esac
     shift
   done
   ```

3. **平台配置模块**
   ```bash
   # 目标平台映射
   case "$TARGET_INDEX" in
   0) # 主机平台
      if [ "$HOST_OS" == "mac" ]; then
          BUILD_TARGET_OS="mac"
          BUILD_TARGET_ARCH="x86_64"
      fi
      ;;
   2) # Android ARM64
      BUILD_TARGET_OS="android"
      BUILD_TARGET_ARCH="arm64-v8a"
      ;;
   esac
   ```

4. **工具链配置模块**
   ```bash
   # 自动配置工具链
   if [[ $TARGET == *"android"* ]]; then
     if [ "$ANDROID_NDK_HOME" = "" ]; then
         echo "ANDROID_NDK_HOME 未设置!!!"
         exit 0
     fi
     TARGET_TOOLCHAIN=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
   fi
   ```

### 3.2 build-android-armv8a-release.sh Android 构建脚本

```bash
#!/usr/bin/env bash
# Android ARM64 Release 版本构建脚本
#
# 功能：
# 1. 自动下载 OpenCV 源码和扩展模块
# 2. 解压源码到指定目录
# 3. 调用主构建脚本进行编译
#
# 构建流程：
# 下载 -> 解压 -> 配置 -> 编译 -> 安装
```

**主要功能：**

1. **源码下载模块**
   ```bash
   # OpenCV 主源码下载
   if [ ! -e "$ZIP_OPENCV" ]; then
       echo "开始下载 OpenCV $TARGET_VERSION"
       wget -O "$ZIP_OPENCV" "$URL_TARGET_ZIP"
   fi
   
   # OpenCV Contrib 扩展模块下载
   if [ ! -e "$ZIP_OPENCV_CONTRIB" ]; then
       echo "开始下载 OpenCV Contrib $TARGET_VERSION"
       wget -O "$ZIP_OPENCV_CONTRIB" "$URL_TARGET_CONTRIB_ZIP"
   fi
   ```

2. **源码解压模块**
   ```bash
   # 解压 OpenCV 主源码
   if [ ! -d "$TARGET_DL_DIR" ]; then
       echo "解压 OpenCV 源码: $ZIP_OPENCV"
       unzip -q "$ZIP_OPENCV" -d "$BASH_PATH"
   fi
   
   # 解压 Contrib 扩展模块
   if [ ! -d "$TARGET_CONTRIB_DL_DIR" ]; then
       echo "解压 OpenCV Contrib: $ZIP_OPENCV_CONTRIB"
       unzip -q "$ZIP_OPENCV_CONTRIB" -d "$BASH_PATH"
   fi
   ```

3. **构建调用模块**
   ```bash
   # 调用主构建脚本
   ./build.sh -t 2 -r -n "$TARGET_NAME" -v "$TARGET_VERSION" \
              -a $(cat ./cmake_args/common_options.txt) \
              -i "$DIR_INSTALL"
   ```

## 4. 📦 构建产物说明

### 4.1 静态库文件 (16 个核心模块)

| 库文件名 | 大小 | 功能说明 |
|---------|------|---------|
| `libopencv_core.a` | 29.1 MB | 核心功能模块 |
| `libopencv_imgproc.a` | 32.7 MB | 图像处理 |
| `libopencv_dnn.a` | 106.5 MB | 深度神经网络 |
| `libopencv_calib3d.a` | 27.7 MB | 相机标定和 3D 重建 |
| `libopencv_features2d.a` | 9.8 MB | 2D 特征框架 |
| `libopencv_objdetect.a` | 16.4 MB | 目标检测 |
| `libopencv_video.a` | 7.1 MB | 视频分析 |
| `libopencv_videoio.a` | 4.7 MB | 视频 I/O |
| `libopencv_highgui.a` | 2.1 MB | 高级 GUI |
| `libopencv_imgcodecs.a` | 7.4 MB | 图像编解码 |
| `libopencv_ml.a` | 7.4 MB | 机器学习 |
| `libopencv_photo.a` | 6.1 MB | 计算摄影 |
| `libopencv_stitching.a` | 7.6 MB | 图像拼接 |
| `libopencv_flann.a` | 6.4 MB | 快速近似最近邻 |
| `libopencv_gapi.a` | 126.9 MB | 图 API |
| `libopencv_ts.a` | 9.2 MB | 测试支持 |

### 4.2 构建配置详情

**目标平台:**
- **架构**: Android ARM64 (armeabi-v7a)
- **OpenCV 版本**: 4.11.0
- **Android NDK**: r26d (Clang 17.0.2)
- **API Level**: 21 (Android 5.0)
- **构建类型**: Release

**启用的功能:**
- ✅ NEON SIMD 优化
- ✅ MediaNDK 支持
- ✅ Java 绑定生成
- ✅ 多图像格式支持 (JPEG, PNG, TIFF, WebP, OpenEXR)
- ✅ DNN 模块 (Protobuf 3.19.1)
- ✅ 并行处理支持 (OpenMP, TBB)

## 5. 🚀 快速开始

### 5.1 环境准备

```bash
# 设置 Android 开发环境
export ANDROID_NDK_HOME=/home/frewen/snap/Android/ndk/android-ndk-r26d
export ANDROID_SDK_ROOT=/home/frewen/Android/Sdk
export ANDROID_HOME=/home/frewen/Android/Sdk

# 验证环境变量
echo "Android NDK: $ANDROID_NDK_HOME"
echo "Android SDK: $ANDROID_SDK_ROOT"
```

### 5.2 执行构建

```bash
# 进入构建目录
cd /home/frewen/01.WorkSpace/AuraKaleidos/FantasyCXX/3rdparty/opencv/compiler

# 执行 Android ARM64 构建
./scripts/build-android-armv8a-release.sh
```

### 5.3 验证构建结果

```bash
# 检查生成的库文件
ls -la build/android-arm64-v8a-release/lib/armeabi-v7a/

# 检查测试程序
ls -la build/android-arm64-v8a-release/bin/

# 检查头文件
ls -la build/android-arm64-v8a-release/include/
```

## 6. 📋 常见问题解答 (FAQ)

### 6.1 构建问题

**Q1: 构建时提示 "ANDROID_NDK_HOME is not set"**
```bash
# 解决方案
export ANDROID_NDK_HOME=/path/to/your/ndk
export PATH=$ANDROID_NDK_HOME:$PATH
```

**Q2: CMake 配置失败**
```bash
# 检查 Android SDK 路径
export ANDROID_SDK_ROOT=/path/to/android/sdk
export ANDROID_HOME=/path/to/android/sdk
```

**Q3: 内存不足导致构建失败**
```bash
# 减少并行编译线程数
# 在 build.sh 中修改:
threads=$(( $cpu - 4 ))  # 原来是 -2
```

### 6.2 运行时问题

**Q1: Java 加载 OpenCV 库失败**
```java
// 确保正确加载库
static {
    System.loadLibrary("opencv_java4");
}
```

## 7. 📚 相关文档

- [编译指南](compilation_guide.md) - 详细的编译步骤和配置说明
- [测试指南](testing_guide.md) - 测试程序使用方法和测试案例
- [示例代码](examples/) - 完整的代码示例和使用方法

---

**构建完成！您现在可以使用生成的 OpenCV Android 库进行开发了！** 🎉