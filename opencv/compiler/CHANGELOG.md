# OpenCV 构建系统更新日志

## v3.0 (2026-05-10)

### 🎯 重大更新

#### 1. 重构项目结构
- **新结构**: 将所有构建逻辑移到 `compiler/` 目录中
- **清晰分离**: 通用构建脚本与 OpenCV 专用脚本分离
- **文档完善**: 详细的 README 和使用说明

#### 2. 通用构建脚本 (build.sh)
- ✅ 完全重写，可复用于任何 C++ 项目
- ✅ 支持 Android、macOS、iOS、Linux、Windows 平台
- ✅ 灵活的配置选项
- ✅ 统一的输出结构
- ✅ 完善的错误检查

#### 3. OpenCV 专用脚本 (build-opencv.sh)
- ✅ 自动下载 OpenCV 源码功能
- ✅ 支持 opencv_contrib 模块
- ✅ 多平台并行构建
- ✅ 最小化构建选项
- ✅ CUDA 加速支持
- ✅ 详细的构建选项配置

### 📁 新目录结构

```
3rdparty/opencv/
├── compiler/
│   ├── build.sh              # 🎯 通用 C++ 构建脚本
│   ├── build-opencv.sh       # 🎯 OpenCV 专用构建脚本
│   ├── README.md             # 详细文档
│   └── CHANGELOG.md          # 本文档
├── opencv.cmake              # FantasyCXX 集成配置
├── lib/v4.11.0/             # 构建输出目录
│   ├── android-arm64-v8a-release/
│   ├── mac-x86_64-release/
│   ├── linux-x86_64-release/
│   └── ios-arm64-release/
└── README.md                # 快速开始指南
```

### 🚀 新功能

#### 自动源码下载
```bash
# 仅下载源码
./build-opencv.sh download

# 构建时自动下载
./build-opencv.sh android
```

#### 多平台支持
```bash
# 构建单个平台
./build-opencv.sh android
./build-opencv.sh mac
./build-opencv.sh ios

# 构建所有平台
./build-opencv.sh all
```

#### 构建选项
```bash
# 最小化构建
./build-opencv.sh android --minimal

# 包含 contrib 模块
./build-opencv.sh android --with-contrib

# 启用 CUDA
./build-opencv.sh linux --with-cuda

# 调试版本
./build-opencv.sh mac -c debug
```

### 🔧 技术改进

#### 1. 错误处理
- 完善的命令检查
- 环境验证
- 详细的错误信息

#### 2. 性能优化
- 自动检测 CPU 核心数
- 并行构建支持
- ccache 自动启用

#### 3. 平台兼容性
- Android NDK 支持
- iOS Xcode 工具链
- Windows MinGW 支持
- 跨平台路径处理

### 📋 使用示例

#### 快速开始
```bash
cd compiler

# 设置环境
export ANDROID_NDK_HOME="/path/to/android-ndk"

# 构建 Android 版本
./build-opencv.sh android

# 构建所有平台
./build-opencv.sh all
```

#### 高级用法
```bash
# 自定义线程数
export BUILD_THREADS=8

# 详细输出
export VERBOSE=true

# CUDA 支持
export OPENCV_CUDA_ARCH_BIN="7.5"
./build-opencv.sh linux --with-cuda
```

### 🔄 向后兼容性

- 保持原有的输出目录结构
- 兼容现有的 CMake 集成配置
- 支持原有的环境变量

### 📚 文档改进

- 详细的 README 文档
- 清晰的使用说明
- 故障排除指南
- 构建配置说明

### 🎯 设计原则

1. **简洁性**: 脚本简单易懂，避免过度复杂化
2. **通用性**: `build.sh` 可复用于任何 C++ 项目
3. **专用性**: `build-opencv.sh` 针对 OpenCV 优化
4. **健壮性**: 完善的错误检查和环境验证
5. **文档化**: 清晰的文档和使用说明

### 🔮 未来计划

- [ ] 支持更多 OpenCV 版本
- [ ] 添加 CI/CD 集成示例
- [ ] 支持更多嵌入式平台
- [ ] 添加性能基准测试
- [ ] 支持模块化构建配置

---

**🎉 总结**: v3.0 版本完全重构了 OpenCV 构建系统，提供了更清晰的结构、更强大的功能和更好的用户体验。通用构建脚本现在可以复用于其他 C++ 项目，OpenCV 专用脚本则提供了完整的自动化构建流程。