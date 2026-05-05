# OpenCV 构建系统 - 快速开始指南

## 🎯 5分钟快速开始

### 步骤 1: 进入构建目录

```bash
cd 3rdparty/opencv/compiler
```

### 步骤 2: 设置环境变量

```bash
# Android 构建必需
export ANDROID_NDK_HOME="/path/to/android-ndk"

# 可选：设置并行构建线程数
export BUILD_THREADS=8
```

### 步骤 3: 构建 OpenCV

#### 构建 Android 版本
```bash
./build-opencv.sh android
```

#### 构建 macOS 版本
```bash
./build-opencv.sh mac
```

#### 构建所有平台
```bash
./build-opencv.sh all
```

### 步骤 4: 使用构建结果

构建完成后，库文件位于：
```
../lib/v4.11.0/
├── android-arm64-v8a-release/
├── mac-x86_64-release/
└── linux-x86_64-release/
```

在你的 CMake 项目中使用：
```cmake
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/3rdparty/opencv/lib/v4.11.0/android-arm64-v8a-release/share/opencv4")
find_package(OpenCV REQUIRED)
target_link_libraries(your_target ${OpenCV_LIBS})
```

## 🚀 常用命令

| 命令 | 说明 |
|------|------|
| `./build-opencv.sh android` | 构建 Android 版本 |
| `./build-opencv.sh mac` | 构建 macOS 版本 |
| `./build-opencv.sh all` | 构建所有平台 |
| `./build-opencv.sh android --minimal` | 构建最小化 Android 版本 |
| `./build-opencv.sh clean` | 清理构建文件 |
| `./build-opencv.sh help` | 显示帮助信息 |

## 🔧 高级选项

### 构建配置
```bash
# 调试版本
./build-opencv.sh android -c debug

# 自定义线程数
./build-opencv.sh android -j 8

# 包含 opencv_contrib
./build-opencv.sh android --with-contrib

# 启用 CUDA
./build-opencv.sh linux --with-cuda
```

### 环境变量
```bash
# 详细输出
export VERBOSE=true

# CUDA 架构
export OPENCV_CUDA_ARCH_BIN="7.5"
```

## 📖 更多文档

- [详细文档](compiler/README.md) - 完整的使用说明和配置选项
- [更新日志](compiler/CHANGELOG.md) - 版本更新历史
- [项目 README](../README.md) - 项目概览

## 🐛 常见问题

### Q: Android 构建失败？
A: 确保设置了 `ANDROID_NDK_HOME` 环境变量

### Q: 构建速度慢？
A: 设置 `BUILD_THREADS=$(nproc)` 使用最大线程数

### Q: 如何清理构建？
A: 运行 `./build-opencv.sh clean`

## 🎯 设计特点

- ✅ **简洁**: 易于理解和使用
- ✅ **通用**: build.sh 可复用于其他 C++ 项目
- ✅ **专用**: build-opencv.sh 针对 OpenCV 优化
- ✅ **自动**: 自动下载源码和依赖
- ✅ **灵活**: 支持多种构建配置

---

**💡 提示**: 建议先运行 `./build-opencv.sh help` 查看完整的选项说明。