# CLAUDE.md

本文档为 Claude Code (claude.ai/code) 提供在此 OpenCV 构建系统目录中工作的指导。

## OpenCV 构建系统概述

此目录包含专为 FantasyCXX 项目设计的跨平台 OpenCV 构建系统。它为 Android、macOS、Linux 和 Windows 平台提供优化的 OpenCV 构建。

## 快速构建命令

### 主要构建脚本（推荐）

```bash
cd compiler

# 构建 Android 版本 (arm64-v8a)
./build-opencv.sh android

# 构建 macOS 版本
./build-opencv.sh mac

# 构建 Linux 版本
./build-opencv.sh linux

# 构建所有支持的平台
./build-opencv.sh all

# 清理构建产物
./build-opencv.sh clean
```

### 通用构建脚本（可复用）

```bash
cd compiler

# 使用调试配置构建 Android
./build.sh build -t android -c debug

# 使用发布配置构建 macOS
./build.sh build -t mac -c release

# 清理构建
./build.sh clean
```

## 高级构建选项

### OpenCV 专用选项

```bash
# 调试版本构建
./build-opencv.sh android -c debug

# 自定义线程数
./build-opencv.sh android -j 8

# 最小化构建（仅核心模块）
./build-opencv.sh android --minimal

# 包含 opencv_contrib 模块
./build-opencv.sh android --with-contrib

# 启用 CUDA 支持（Linux）
./build-opencv.sh linux --with-cuda
```

### 环境变量

```bash
# Android 构建必需
export ANDROID_NDK_HOME="/path/to/android-ndk"

# 并行构建线程数（可选）
export BUILD_THREADS=8

# 详细输出
export VERBOSE=true

# CUDA 架构版本
export OPENCV_CUDA_ARCH_BIN="7.5"
```

## 构建输出结构

构建成功后，库文件按以下结构组织：
```
lib/v4.11.0/
├── android-arm64-v8a-release/    # Android 库文件
├── mac-x86_64-release/           # macOS 库文件
├── linux-x86_64-release/         # Linux 库文件
└── windows-x86_64-release/       # Windows 库文件
```

## 与 CMake 项目集成

### Android 集成

```cmake
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/3rdparty/opencv/lib/v4.11.0/android-arm64-v8a-release/share/opencv4")
find_package(OpenCV REQUIRED)
target_link_libraries(your_target ${OpenCV_LIBS})
```

### macOS/Linux 集成

```cmake
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/3rdparty/opencv/lib/v4.11.0/mac-x86_64-release/share/opencv4")
find_package(OpenCV REQUIRED)
target_link_libraries(your_target ${OpenCV_LIBS})
```

## 平台支持矩阵

| 平台 | 架构 | 状态 | 说明 |
| :--- | :--- | :--- | :--- |
| Android | arm64-v8a | ✅ 支持 | 需要 NDK |
| macOS | x86_64 | ✅ 支持 | 主要开发平台 |
| Linux | x86_64 | ✅ 支持 | 主要开发平台 |
| Windows | x86_64 | ✅ 支持 | 需要 MinGW |

## 目录结构

```text
3rdparty/opencv/
├── compiler/
│   ├── build.sh              # 通用 C++ 构建脚本（可复用）
│   ├── build-opencv.sh       # OpenCV 专用构建脚本
│   └── README.md             # 详细文档
├── opencv.cmake              # FantasyCXX 集成配置
├── lib/                      # 构建输出目录
├── README.md                # 项目概览
├── QUICKSTART.md            # 快速开始指南
└── CLAUDE.md                # 本文档
```

## 关键文件

### 构建脚本
- `compiler/build-opencv.sh`: 针对 OpenCV 优化的专用构建脚本
- `compiler/build.sh`: 通用 C++ 构建脚本（可用于其他项目）

### 配置文件

- `opencv.cmake`: FantasyCXX 项目集成配置
- `compiler/README.md`: 完整的构建文档

### 文档

- `README.md`: 项目概览和使用指南
- `QUICKSTART.md`: 5 分钟快速开始指南

## 故障排除

### 常见问题

1. **找不到 Android NDK**
   ```bash
   # 设置 Android NDK 路径
   export ANDROID_NDK_HOME="/Users/username/Library/Android/sdk/ndk/版本号"
   # 验证设置
   echo $ANDROID_NDK_HOME
   ```

2. **找不到 CMake**
   ```bash
   # macOS
   brew install cmake
   # Ubuntu
   sudo apt install cmake
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

# 显示通用脚本帮助
./build.sh help

# 显示 OpenCV 脚本帮助
./build-opencv.sh help
```

## 开发指南

### 自定义构建选项

编辑 `compiler/build-opencv.sh` 脚本中的 `generate_cmake_options()` 函数来自定义 OpenCV 构建选项。

### 复用通用构建脚本

`compiler/build.sh` 脚本设计为可复用：
```bash
# 复制到其他 C++ 项目
cp compiler/build.sh /path/to/your/cpp/project/

# 用于其他项目
./build.sh build -t android -c release
```

### 添加新平台支持

1. 在 `compiler/build.sh` 中添加新的构建函数
2. 在 `build_project()` 函数中添加对应的 case 分支
3. 实现平台特定的构建逻辑

## 设计原则

1. **简洁性**: 脚本易于理解和维护
2. **通用性**: `build.sh` 可复用于任何 C++ 项目
3. **专用性**: `build-opencv.sh` 针对 OpenCV 优化
4. **健壮性**: 完善的错误检查和验证
5. **文档化**: 清晰的文档和使用示例

## OpenCV 版本

此构建系统针对 OpenCV 版本 4.11.0，为移动和桌面平台提供优化配置。

## 集成说明

- 此 OpenCV 构建专门为 FantasyCXX 项目配置
- `opencv.cmake` 文件提供集成配置
- 构建的库自动放置在 `lib/` 目录
- 支持最小化和完整 OpenCV 构建
