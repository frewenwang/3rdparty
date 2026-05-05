# OpenCV 跨平台构建系统

🎯 **简洁、通用、易复用的 OpenCV 构建解决方案**

> 📦 本仓库提供了优化的 OpenCV 跨平台构建系统，支持 Android、macOS、Linux 等多个平台。
> 包含通用构建脚本可复用于其他 C++ 项目。

## 📋 项目结构

```text
3rdparty/opencv/
├── compiler/
│   ├── build.sh              # 🎯 通用 C++ 构建脚本 (可复用)
│   ├── build-opencv.sh       # 🎯 OpenCV 专用构建脚本
│   └── README.md             # 详细文档
├── opencv.cmake              # FantasyCXX 集成配置
├── lib/                      # 构建输出目录
└── README.md                # 本文档
```

## 🚀 快速开始

### 方法1: 使用统一构建脚本 (推荐)

```bash
cd compiler

# 构建 Android 版本（自动统一目录结构）
./build-unified.sh android

# 构建 macOS 版本（自动统一目录结构）
./build-unified.sh mac

# 构建所有平台（自动统一目录结构）
./build-unified.sh all

# 清理构建
./build-opencv.sh clean
```

### 方法2: 使用通用构建脚本

```bash
cd compiler

# 构建 Android
./build.sh build -t android -c release

# 构建 macOS
./build.sh build -t mac -c release

# 清理
./build.sh clean
```

## 🛠️ 构建选项

### OpenCV 专用脚本选项

```bash
cd compiler

# 构建调试版本
./build-opencv.sh android -c debug

# 使用 8 个线程并行构建
./build-opencv.sh android -j 8

# 最小化构建 (仅核心模块)
./build-opencv.sh android --minimal

# 启用 opencv_contrib 模块
./build-opencv.sh android --with-contrib

# 启用 CUDA 支持
./build-opencv.sh linux --with-cuda
```

### 通用脚本选项

```bash
cd compiler

# 指定构建配置
./build.sh build -t android -c debug

# 指定并行线程数
./build.sh build -t mac -j 8
```

## 📱 支持平台

| 平台    | 架构        | 状态           |
|---------|-------------|----------------|
| Android | arm64-v8a   | ✅ 支持        |
| macOS   | x86_64      | ✅ 支持        |
| Linux   | x86_64      | ✅ 支持        |
| Windows | x86_64      | ✅ 支持 (MinGW) |

## ⚙️ 环境要求

### 通用要求

- CMake 3.10+
- Make/GCC
- Git

### Android 构建

```bash
# 设置 Android NDK 路径
export ANDROID_NDK_HOME="/path/to/android-ndk"

# 验证设置
echo $ANDROID_NDK_HOME
```

### 可选环境变量

```bash
# 设置并行构建线程数
export BUILD_THREADS=8

# 启用详细输出
export VERBOSE=true

# CUDA 架构版本
export OPENCV_CUDA_ARCH_BIN="7.5"
```

## 📦 构建输出

构建完成后，库文件位于：

```text
lib/v4.11.0/
├── android-arm64-v8a-release/    # Android 库文件
├── mac-x86_64-release/           # macOS 库文件
└── linux-x86_64-release/         # Linux 库文件
```

### 在 CMake 项目中使用

```cmake
# Android 平台
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/3rdparty/opencv/lib/v4.11.0/android-arm64-v8a-release/share/opencv4")

# macOS 平台  
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/3rdparty/opencv/lib/v4.11.0/mac-x86_64-release/share/opencv4")

# 查找 OpenCV
find_package(OpenCV REQUIRED)

# 链接库
target_link_libraries(your_target ${OpenCV_LIBS})
```

## 🔧 高级用法

### 自定义构建选项

编辑 `compiler/build-opencv.sh` 脚本中的 `generate_cmake_options()` 函数来自定义构建选项。

### 复用通用构建脚本

`compiler/build.sh` 是一个通用的 C++ 跨平台构建脚本，可复用于其他项目：

```bash
# 复制到你的 C++ 项目
cp compiler/build.sh /path/to/your/cpp/project/

# 使用示例
./build.sh build -t android -c release
```

### 扩展新平台支持

1. 在 `compiler/build.sh` 中添加新的构建函数
2. 在 `build_project()` 函数中添加对应的 case 分支
3. 实现平台特定的构建逻辑

## 🐛 故障排除

### 常见问题

1. **CMake 未找到**

   ```bash
   brew install cmake    # macOS
   sudo apt install cmake # Ubuntu
   ```

2. **Android NDK 路径错误**

   ```bash
   export ANDROID_NDK_HOME="/Users/yourname/Library/Android/sdk/ndk/版本号"
   ```

3. **构建失败**

   ```bash
   cd compiler
   # 清理后重试
   ./build-opencv.sh clean
   ./build-opencv.sh android
   ```

### 获取帮助

```bash
cd compiler

# 查看通用脚本帮助
./build.sh help

# 查看 OpenCV 脚本帮助
./build-opencv.sh help
```

## 📖 详细文档

- [compiler/README.md](compiler/README.md) - 完整的使用说明和高级配置选项
- [QUICKSTART.md](QUICKSTART.md) - 5分钟快速开始指南
- [compiler/CHANGELOG.md](compiler/CHANGELOG.md) - 版本更新历史

## 🎯 设计原则

1. **简洁性**: 脚本简单易懂，避免过度复杂化
2. **通用性**: `build.sh` 可复用于任何 C++ 项目
3. **专用性**: `build-opencv.sh` 针对 OpenCV 优化
4. **健壮性**: 完善的错误检查和环境验证
5. **文档化**: 清晰的文档和使用说明

## 📄 许可证

本项目遵循 OpenCV 的许可证条款。

## 🎯 统一目录结构解决方案

### 问题描述

原始 OpenCV 构建在不同平台上产生不同的安装目录结构：
- **macOS**: 使用标准 CMake 安装结构
- **Android**: 使用 Android NDK 特定的 SDK 结构

### 解决方案

我们提供了 `build-unified.sh` 脚本，自动统一安装目录结构：

#### 统一后的目录结构

```text
lib/v4.11.0/
├── android-arm64-v8a-release/
│   ├── include/opencv4/     # 头文件
│   ├── lib/                 # 主要库文件
│   │   └── opencv4/3rdparty/ # 第三方库
│   └── share/opencv4/       # CMake 配置
└── mac-x86_64-release/
    ├── include/opencv4/     # 头文件
    ├── lib/                 # 主要库文件
    │   └── opencv4/3rdparty/ # 第三方库
    └── share/opencv4/       # CMake 配置
```

#### 使用方式
```bash
# 使用统一构建脚本（推荐）
./build-unified.sh android  # 自动构建并统一结构
./build-unified.sh mac      # 自动构建并统一结构
./build-unified.sh all      # 构建所有平台并统一结构
```

#### 集成配置
统一结构后，CMake 集成更加简单：
```cmake
# 自动适配平台
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/3rdparty/opencv/lib/v4.11.0/${TARGET_OS}-${TARGET_ARCH}-release/share/opencv4")
find_package(OpenCV REQUIRED)
target_link_libraries(your_target ${OpenCV_LIBS})
```

---

**💡 提示**: 使用 `build-unified.sh` 可以自动处理目录结构统一，便于嵌入式跨平台构建配置。
