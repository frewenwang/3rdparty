# OpenCV 跨平台构建系统 v3.0

这是一个专为 FantasyCXX 项目优化的 OpenCV 跨平台构建系统，提供了两个互补的构建脚本：

- **build.sh**: 通用 C++ 跨平台构建脚本，可复用于任何 C++ 项目
- **build-opencv.sh**: 专门的 OpenCV 构建脚本，支持自动下载源码和多平台构建

## 🎯 主要特性

### 通用构建脚本 (build.sh)
- ✅ 支持 Android、macOS、iOS、Linux、Windows 平台
- ✅ 可复用于任何 CMake 基础的 C++ 项目
- ✅ 统一的输出结构
- ✅ 灵活的配置选项

### OpenCV 专用脚本 (build-opencv.sh)
- ✅ 自动从 GitHub 下载 OpenCV 源码
- ✅ 支持 opencv_contrib 模块
- ✅ 多平台并行构建
- ✅ 最小化构建选项
- ✅ CUDA 加速支持

## 📁 目录结构

```
compiler/
├── build.sh                    # 🎯 通用 C++ 构建脚本
├── build-opencv.sh             # 🎯 OpenCV 专用构建脚本
├── README.md                   # 本文档
├── build/                      # 构建临时目录
└── ../lib/v4.11.0/            # 构建输出目录
    ├── android-arm64-v8a-release/
    ├── mac-x86_64-release/
    ├── linux-x86_64-release/
    └── ios-arm64-release/
```

## 🚀 快速开始

### 1. 环境准备

```bash
# 设置 Android NDK (Android 构建必需)
export ANDROID_NDK_HOME="/path/to/android-ndk"

# 可选：设置并行构建线程数
export BUILD_THREADS=8

# 可选：启用详细输出
export VERBOSE=true
```

### 2. 使用 OpenCV 专用脚本

```bash
# 构建 Android ARM64 发布版本
./build-opencv.sh android

# 构建所有支持的平台
./build-opencv.sh all

# 构建最小化版本
./build-opencv.sh android --minimal

# 构建包含 opencv_contrib 的版本
./build-opencv.sh android --with-contrib

# 构建调试版本
./build-opencv.sh mac -c debug

# 仅下载源码（不构建）
./build-opencv.sh download

# 清理构建文件
./build-opencv.sh clean
```

### 3. 使用通用构建脚本

```bash
# 构建特定平台
./build.sh build -t android -c release
./build.sh build -t mac -c debug -j 8

# 指定自定义项目目录
./build.sh build -t linux -p ./src -o ./output

# 清理构建文件
./build.sh clean
```

## 📋 构建选项说明

### OpenCV 脚本选项

| 选项 | 说明 |
|------|------|
| `-c, --config` | 构建配置 (debug/release) |
| `-j, --jobs` | 并行构建线程数 |
| `--minimal` | 最小化构建（仅核心模块） |
| `--with-contrib` | 包含 opencv_contrib 模块 |
| `--with-cuda` | 启用 CUDA 支持 |
| `--skip-download` | 跳过源码下载 |

### 平台支持

| 平台 | 标识符 | 架构 |
|------|--------|------|
| Android | `android` | arm64-v8a |
| macOS | `mac` | x86_64 |
| iOS | `ios` | arm64 |
| Linux | `linux` | x86_64 |
| Windows | `windows` | x86_64 |

## 🔧 环境变量

| 变量名 | 说明 | 必需 |
|--------|------|------|
| `ANDROID_NDK_HOME` | Android NDK 路径 | Android 构建必需 |
| `BUILD_THREADS` | 并行构建线程数 | 否 |
| `VERBOSE` | 启用详细输出 | 否 |
| `OPENCV_CUDA_ARCH_BIN` | CUDA 架构版本 | CUDA 构建时可选 |

## 📦 输出结构

构建成功后，库文件按以下方式组织：

```
lib/v4.11.0/
├── android-arm64-v8a-release/
│   ├── lib/                    # 静态库文件
│   ├── include/opencv4/        # 头文件
│   └── share/opencv4/         # CMake 配置文件
├── mac-x86_64-release/
│   ├── lib/
│   ├── include/opencv4/
│   └── share/opencv4/
└── linux-x86_64-release/
    ├── lib/
    ├── include/opencv4/
    └── share/opencv4/
```

## 🔗 集成到 CMake 项目

### Android 项目

```cmake
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/3rdparty/opencv/lib/v4.11.0/android-arm64-v8a-release/share/opencv4")
find_package(OpenCV REQUIRED)
target_link_libraries(your_target ${OpenCV_LIBS})
```

### macOS/Linux 项目

```cmake
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/3rdparty/opencv/lib/v4.11.0/mac-x86_64-release/share/opencv4")
find_package(OpenCV REQUIRED)
target_link_libraries(your_target ${OpenCV_LIBS})
```

## 🎯 构建配置说明

### 最小化构建 (--minimal)
禁用非核心模块，减小输出体积：
- ❌ DNN (深度学习)
- ❌ ML (机器学习)
- ❌ Photo (图像处理)
- ❌ Video (视频处理)
- ❌ VideoIO (视频 I/O)
- ❌ HighGUI (GUI 界面)
- ❌ ImgCodecs (图像编解码)
- ❌ ObjDetect (目标检测)
- ❌ Features2D (特征检测)
- ❌ Flann (快速近似最近邻)
- ❌ Calib3D (相机标定)

### 完整构建 (默认)
包含所有核心模块：
- ✅ Core (核心功能)
- ✅ ImgProc (图像处理)
- ✅ 其他所有模块

## 🚀 性能优化

### 构建速度优化
```bash
# 使用最大 CPU 线程数
export BUILD_THREADS=$(nproc)

# 启用 ccache (自动配置)
# 在 CMake 选项中自动启用
```

### 输出大小优化
```bash
# 生产环境使用最小化构建
./build-opencv.sh android --minimal

# 静态链接减小最终二进制文件大小
# 默认配置为静态库
```

## 🔍 故障排除

### 常见问题

1. **找不到 Android NDK**
   ```bash
   export ANDROID_NDK_HOME="/path/to/android-ndk"
   ```

2. **缺少 CMake**
   ```bash
   # Ubuntu/Debian
   sudo apt install cmake
   
   # macOS
   brew install cmake
   ```

3. **构建失败**
   ```bash
   # 清理后重试
   ./build-opencv.sh clean
   ./build-opencv.sh android
   ```

4. **构建缓慢**
   ```bash
   # 增加并行线程数
   export BUILD_THREADS=$(nproc)
   ```

### 诊断命令

```bash
# 检查环境
./build-opencv.sh help

# 启用详细输出
export VERBOSE=true
./build-opencv.sh android

# 清理并重新构建
./build-opencv.sh clean
./build-opencv.sh android
```

## 📝 版本管理

- **当前版本**: OpenCV 4.11.0
- **版本切换**: 修改构建脚本中的 `OPENCV_VERSION` 变量
- **源码管理**: 自动下载和解压，无需手动管理

## 🎨 设计原则

1. **简洁性**: 脚本简单明了，文档完善
2. **可复用性**: 通用 `build.sh` 可复制到其他 C++ 项目
3. **优化**: 针对性能优化的平台特定配置
4. **集成**: 通过 CMake 实现与 FantasyCXX 项目的无缝集成
5. **可维护性**: 清晰的结构和完整的文档

## 🤝 贡献指南

### 添加新平台支持

1. 在 `build.sh` 中添加平台检测
2. 实现平台特定的构建函数
3. 在 `build-opencv.sh` 中添加相应的 OpenCV 配置
4. 更新文档和帮助信息

### 自定义构建选项

修改 `build-opencv.sh` 中的 `generate_cmake_options()` 函数来添加项目特定的 CMake 标志。

## 📞 支持

如遇问题，请参考：
- 本文档中的故障排除部分
- CMake 官方文档
- OpenCV 官方构建指南

---

**🎯 提示**: 首次使用时建议先运行 `./build-opencv.sh download` 下载源码，然后再进行构建。