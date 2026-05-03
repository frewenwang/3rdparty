# OpenCV 构建系统 - 中文说明

这是一个为 FantasyCXX 项目优化的 OpenCV 构建系统，支持多平台交叉编译和定制化构建。

## 🚀 快速开始

### 前置要求
- CMake 3.10+
- GNU Make
- 对应平台的 SDK（Android NDK、iOS SDK 等）

### 基本使用

```bash
# 进入构建目录
cd compiler

# 构建当前平台
./build.sh

# 构建 Android ARM64
./build.sh -t 2 -r -n opencv -v 4.11.0

# 查看帮助
./build.sh --help
```

## 🎯 支持的平台

| 索引 | 平台 | 架构 | 描述 |
|------|------|------|------|
| 0 | 宿主平台 | 自动检测 | 当前操作系统 |
| 1 | Android | armeabi-v7a | Android ARMv7 |
| 2 | Android | arm64-v8a | Android ARM64 ⭐ |
| 3 | Android | x86 | Android x86 |
| 4 | Android | x86_64 | Android x86_64 |
| 5 | QNX | armv7le | QNX ARMv7 |
| 6 | QNX | aarch64le | QNX ARM64 |
| 7 | iOS | armv7 | iOS ARMv7 |
| 8 | iOS | arm64 | iOS ARM64 ⭐ |
| 9 | AX620A | arm7 | 自定义嵌入式平台 |

## 📁 项目结构

```
opencv/
├── README_中文.md              # 本文档（中文说明）
├── 快速开始.md                 # 快速入门指南
├── 工程结构.md                 # 详细项目结构
├── 文档索引.md                 # 完整文档索引
├── opencv.cmake               # CMake 集成文件
│
├── compiler/                  # 构建系统核心
│   ├── build.sh               # 主构建脚本
│   ├── cmake_args/            # 平台特定配置
│   │   ├── android_options.txt
│   │   ├── host_options.txt
│   │   ├── embedded_options.txt
│   │   ├── android_配置说明.txt
│   │   ├── host_配置说明.txt
│   │   └── embedded_配置说明.txt
│   ├── docs/                 # 详细文档
│   └── cmake/                # 工具链配置
│
├── build/                     # 构建输出（自动生成）
└── lib/                       # 库文件输出（自动生成）
```

## 🛠️ 构建选项

### 构建类型
- `-r, --release` - Release 版本（默认，优化）
- `-d, --debug` - Debug 版本（调试符号）
- `--RelWithDebInfo` - 带调试信息的 Release 版本

### 配置选项
- `-t, --target` - 目标平台索引
- `-n, --name` - 构建名称（如 'opencv'）
- `-v, --version` - 版本号（如 '4.11.0'）
- `-i, --install` - 安装路径
- `-a, --args` - 额外的 CMake 参数

## 📱 平台特定配置

### Android 平台
- 使用 `android_options.txt` 配置
- 优化移动应用：禁用不必要的功能，减小体积
- 启用 Android 特定功能（MediaNDK 等）
- 详细配置说明见 `android_配置说明.txt`

### 宿主平台（macOS/Linux）
- 使用 `host_options.txt` 配置
- 启用完整功能集，适合开发和测试
- 支持所有图像格式和视频处理
- 详细配置说明见 `host_配置说明.txt`

### 嵌入式平台
- 使用 `embedded_options.txt` 配置
- 最小化配置，优化体积和性能
- 适用于 QNX、iOS、AX620A 等平台
- 详细配置说明见 `embedded_配置说明.txt`

## 🔧 高级用法

### 自定义 CMake 参数
```bash
# 启用 CUDA 支持
./build.sh -t 0 -r -a "-DWITH_CUDA=ON -DWITH_CUDNN=ON"

# 禁用特定模块
./build.sh -t 2 -r -a "-DBUILD_opencv_dnn=OFF"

# 启用额外功能
./build.sh -t 0 -r -a "-DWITH_OPENCL=ON -DWITH_TBB=ON"
```

### 环境变量

#### Android 构建
```bash
export ANDROID_NDK_HOME="/path/to/android-ndk"
```

#### QNX 构建
```bash
export QNX_HOST="/path/to/qnx/host"
export QNX_TARGET="/path/to/qnx/target"
```

## 📚 文档指南

### 新手推荐阅读顺序
1. [快速开始.md](compiler/docs/快速开始.md) - 5 分钟快速上手
2. [工程结构.md](compiler/docs/工程结构.md) - 了解项目组织
3. 对应平台的配置说明文档
4. 开始第一个构建

### 详细文档
- [文档索引.md](compiler/docs/文档索引.md) - 完整的文档导航
- `compiler/docs/` 目录下的详细指南
- 各平台的配置说明文档

## 🐛 常见问题

### 构建失败
1. 检查是否安装了所有必需的工具
2. 验证环境变量是否正确设置
3. 查看详细的错误输出信息
4. 参考 `compiler/docs/TROUBLESHOOTING.md`

### 找不到工具链
- Android: 确保 `ANDROID_NDK_HOME` 已设置
- QNX: 确保 `QNX_HOST` 和 `QNX_TARGET` 已设置
- iOS: 确保 Xcode 和相关工具已安装

### 磁盘空间不足
- 完整构建可能需要 5-10GB 空间
- 考虑使用精简配置减少体积
- 及时清理不需要的构建目录

## 🎨 优化特性

### 性能优化
- ✅ 平台特定优化配置
- ✅ 自动并行构建
- ✅ 硬件加速支持
- ✅ 智能依赖管理

### 用户体验
- ✅ 详细的日志输出
- ✅ 友好的错误提示
- ✅ 完整的文档支持
- ✅ 灵活的配置选项

### 可维护性
- ✅ 模块化设计
- ✅ 清晰的代码结构
- ✅ 完整的备份机制
- ✅ 持续更新维护

## 🔄 版本历史

### v2.0 (当前版本)
- 重构构建系统，增强错误处理
- 添加平台特定配置
- 完善中文文档
- 优化多平台支持

### v1.0 (原始版本)
- 基础的多平台构建支持
- 简单的配置选项
- 英文文档为主

## 📞 获取帮助

1. **自助解决**
   - 查看 [文档索引.md](文档索引.md)
   - 阅读对应平台的配置说明
   - 参考故障排除文档

2. **深入分析**
   - 启用详细日志输出
   - 分析 CMake 配置过程
   - 检查工具链设置

3. **寻求支持**
   - 参考 OpenCV 官方文档
   - 查看项目相关问题
   - 咨询平台特定社区

## 📄 许可证

本构建系统基于 OpenCV 官方版本进行优化，遵循 OpenCV 的许可证条款。

---

*本中文说明文档会持续更新以反映最新的功能变化。如有疑问或建议，欢迎提出。* 🎉