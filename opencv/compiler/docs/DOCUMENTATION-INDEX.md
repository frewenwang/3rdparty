# OpenCV Android ARM64 编译文档索引

## 📚 文档总览

本目录包含完整的OpenCV Android ARM64编译、验证和测试文档体系。

### 📖 主要文档

| 文档 | 目的 | 目标读者 | 阅读时间 |
|------|------|----------|----------|
| [QUICK-START.md](QUICK-START.md) | 快速开始指南 | 开发者 | 5分钟 |
| [Android-ARM64-Build-Guide.md](Android-ARM64-Build-Guide.md) | 完整编译指南 | 高级用户 | 30分钟 |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | 故障排除指南 | 所有用户 | 按需 |

### 🛠️ 脚本文件

| 脚本 | 功能 | 位置 |
|------|------|------|
| `build-android-arm64-v8a-release-fixed.sh` | ARM64编译脚本 | `compiler/scripts/` |
| `verify_architecture.sh` | 架构验证脚本 | `lib/v4.11.0/android-arm64-v8a-release/` |
| `test_opencv_arm64.py` | Python测试脚本 | 示例代码 |
| `test_opencv.cpp` | C++测试程序 | 示例代码 |

## 📋 文档内容

### 1. 快速开始指南 (QUICK-START.md)

**适用场景**: 快速编译和使用

#### 主要内容
- 🚀 一步编译命令
- 📋 编译前检查清单
- 🔧 编译脚本参数说明
- ✅ 验证编译结果
- 🧪 快速测试方法
- 📁 文件结构说明
- 🔗 项目集成示例
- ⚠️ 常见问题速查

#### 使用流程
```bash
# 1. 环境检查
export ANDROID_NDK_HOME="..."

# 2. 执行编译
./scripts/build-android-arm64-v8a-release-fixed.sh

# 3. 验证结果
./verify_architecture.sh

# 4. 集成到项目
# 参考CMake集成示例
```

### 2. 完整编译指南 (Android-ARM64-Build-Guide.md)

**适用场景**: 深度了解和自定义编译过程

#### 主要内容
- 📖 详细环境准备说明
- 🏗️ 编译脚本完整解析
- ⚙️ CMake参数详细说明
- 🔍 验证脚本使用方法
- 🧪 测试脚本完整实现
- 📝 集成使用示例
- 🐛 故障排除详细步骤
- 🚀 性能优化建议

#### 技术深度
- CMake工具链配置
- Android NDK集成
- 交叉编译原理
- ABI架构说明
- 依赖管理策略

### 3. 故障排除指南 (TROUBLESHOOTING.md)

**适用场景**: 编译和运行时问题解决

#### 主要内容
- 🔧 编译问题解决方案
- ⚙️ 配置问题排查
- 🔗 链接问题处理
- 🏃 运行时问题调试
- ⚡ 性能问题优化
- 🌍 环境问题处理
- 🐞 调试技巧
- 📊 预防性措施

#### 问题分类
- CMake配置错误
- ABI不匹配
- Java环境错误
- 内存不足
- 权限问题
- 跨平台兼容性

## 🎯 使用建议

### 新手用户
1. **第一步**: 阅读 [QUICK-START.md](QUICK-START.md)
2. **第二步**: 执行快速编译
3. **第三步**: 运行验证脚本
4. **第四步**: 集成到项目

### 高级用户
1. **配置**: 参考 [Android-ARM64-Build-Guide.md](Android-ARM64-Build-Guide.md) 自定义参数
2. **优化**: 根据性能需求调整编译选项
3. **调试**: 使用 [TROUBLESHOOTING.md](TROUBLESHOOTING.md) 解决复杂问题

### 开发者
1. **集成**: 参考CMake和Android.mk示例
2. **测试**: 使用提供的测试脚本验证功能
3. **扩展**: 基于编译脚本开发自定义构建流程

## 🔧 环境要求

### 基础环境
- **操作系统**: macOS/Linux/Windows
- **内存**: 8GB+ (推荐16GB+)
- **磁盘空间**: 20GB+ 空闲空间
- **网络**: 用于下载源码和依赖

### 开发工具
- **Android NDK**: r26+ (推荐26.3.11579264)
- **CMake**: 3.10+
- **编译工具**: clang, make
- **辅助工具**: wget, unzip

## 📁 项目结构

```
3rdparty/opencv/
├── Android-ARM64-Build-Guide.md    # 完整编译指南
├── QUICK-START.md                   # 快速开始指南
├── TROUBLESHOOTING.md               # 故障排除指南
├── DOCUMENTATION-INDEX.md           # 本文档
├── compiler/
│   ├── opencv-4.11.0/              # OpenCV源码
│   ├── opencv_contrib-4.11.0/       # 贡献模块
│   └── scripts/
│       ├── build-android-arm64-v8a-release-fixed.sh  # 编译脚本
│       └── build-android-armv8a-release.sh           # 原始脚本
├── lib/v4.11.0/
│   └── android-arm64-v8a-release/  # 编译输出
│       ├── lib/                    # 静态库
│       ├── include/                # 头文件
│       ├── OpenCVConfig.cmake      # CMake配置
│       ├── README.md               # 说明文档
│       └── verify_architecture.sh  # 验证脚本
└── test/                           # 测试代码
    ├── CMakeLists.txt
    ├── test_opencv.cpp
    └── test_opencv_arm64.py
```

## 🔄 工作流程

### 标准编译流程
1. **准备环境** → 设置NDK路径和工具
2. **下载源码** → 自动下载和解压
3. **配置编译** → 设置CMake参数
4. **执行编译** → 构建静态库
5. **验证结果** → 检查架构和文件
6. **集成使用** → 在项目中引用

### 问题排查流程
1. **识别问题** → 查看错误信息
2. **定位原因** → 参考故障排除指南
3. **实施解决** → 执行解决方案
4. **验证修复** → 重新编译测试