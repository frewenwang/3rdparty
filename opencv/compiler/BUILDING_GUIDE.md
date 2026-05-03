# OpenCV Android 构建指南

## 架构概述

本构建系统采用了模块化架构，将构建配置的控制权完全交给调用脚本，而不是在 `build.sh` 中硬编码。

### 主要改进

1. **解耦配置**：`build.sh` 不再硬编码选择特定的选项文件

2. **灵活控制**：调用脚本可以精确控制使用哪些选项文件

3. **完整配置**：默认提供包含最多模块的完整配置

4. **可扩展性**：易于添加自定义配置

## 构建脚本参数

### 新增参数

- `--platform-options FILE`：指定平台特定的选项文件
- `--common-options FILE`：指定通用选项文件（留空字符串表示不使用）

### 示例用法

```bash
# 使用Android完整配置
./build.sh -t 2 -r --platform-options "./cmake_args/android_options.txt" --common-options ""

# 使用自定义选项文件
./build.sh -t 2 -r --platform-options "./my_custom_options.txt" --common-options ""
```

## 完整构建配置

### Release 配置 (`android_options.txt`)

- **用途**：完整配置，包含最大数量的OpenCV模块
- **适用场景**：需要完整OpenCV功能的应用

```bash
./scripts/build-android-arm64-v8a-release.sh
```

## 自定义构建配置

### 创建自定义选项文件

1. 复制现有的配置文件作为模板：
```bash
cp cmake_args/android_standard.txt cmake_args/my_custom.txt
```

2. 编辑自定义选项：
```bash
# 启用DNN模块
-DBUILD_opencv_dnn=ON

# 禁用不需要的模块
-DBUILD_opencv_stitching=OFF
```

3. 使用自定义配置构建：
```bash
./build.sh -t 2 -r \
    --platform-options "./cmake_args/my_custom.txt" \
    --common-options "" \
    -a "-DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21" \
    -i "../../lib/my_custom_build"
```

## 配置选项说明

### 构建类型选项
- `BUILD_SHARED_LIBS`：是否构建动态库（Android通常使用静态库）
- `BUILD_PERF_TESTS`：性能测试
- `BUILD_TESTS`：单元测试
- `BUILD_DOCS`：文档
- `BUILD_EXAMPLES`：示例代码

### 平台特定选项
- `WITH_ANDROID_MEDIANDK`：Android媒体NDK支持
- `WITH_AVFOUNDATION`：Apple AVFoundation（Android禁用）
- `WITH_GTK`：GTK支持（Android禁用）
- `WITH_V4L`：Video4Linux（Android禁用）

### OpenCV模块选项
- `BUILD_opencv_core`：核心模块（必需）
- `BUILD_opencv_imgproc`：图像处理
- `BUILD_opencv_imgcodecs`：图像编解码
- `BUILD_opencv_videoio`：视频I/O
- `BUILD_opencv_calib3d`：3D校准
- `BUILD_opencv_features2d`：2D特征检测
- `BUILD_opencv_flann`：快速近似最近邻
- `BUILD_opencv_ml`：机器学习
- `BUILD_opencv_objdetect`：目标检测
- `BUILD_opencv_photo`：照片处理
- `BUILD_opencv_dnn`：深度神经网络
- `BUILD_opencv_video`：视频分析
- `BUILD_opencv_stitching`：图像拼接

## 故障排除

### 常见问题和解决方案

1. **HPX相关错误**：
   - 确保在Android配置中设置 `-DWITH_HPX=OFF`

2. **平台特定选项错误**：
   - 确保禁用在Android上不支持的选项（如GTK、V4L等）

3. **构建失败**：
   - 检查NDK路径是否正确设置
   - 确保CMake版本兼容

## 性能优化建议

1. **最小化APK大小**：
   - 使用minimal配置
   - 只启用必需的模块
   - 使用静态库而不是动态库

2. **最大化功能**：
   - 使用full配置
   - 启用所有需要的模块

3. **平衡方案**：
   - 使用release配置（完整配置）
   - 根据需要添加或删除特定模块