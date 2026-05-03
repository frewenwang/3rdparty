# OpenCV Android ARM64 快速开始指南

## 🚀 快速编译

### 一步编译
```bash
# 在3rdparty/opencv目录下执行
cd compiler
./scripts/build-android-arm64-v8a-release-fixed.sh
```

### 编译时间
- **首次编译**: 15-30分钟
- **增量编译**: 5-10分钟

## 📋 编译前检查

### 1. 环境验证
```bash
# 检查NDK路径
export ANDROID_NDK_HOME="/Users/frewen/Library/AAura/Android/sdk/ndk/26.3.11579264"
echo "NDK路径: $ANDROID_NDK_HOME"
ls "$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"
```

### 2. 目录结构
```bash
# 确认源码已下载
ls -la compiler/opencv-4.11.0/
ls -la compiler/opencv_contrib-4.11.0/
```

## 🔧 编译脚本参数

### 关键参数说明
| 参数 | 说明 | 推荐值 |
|------|------|--------|
| `-t 2` | 目标架构 | android-armv8a |
| `-r` | 构建类型 | Release |
| `-DANDROID_ABI` | Android ABI | arm64-v8a |
| `-DBUILD_JAVA` | Java支持 | OFF |

### 自定义编译
```bash
# 只编译核心模块
./build.sh -t 2 -r -a "-DBUILD_LIST=core,imgproc,imgcodecs -DBUILD_JAVA=OFF"

# 调试版本
./build.sh -t 2 -d -a "-DBUILD_JAVA=OFF"
```

## ✅ 验证编译结果

### 快速验证
```bash
# 运行验证脚本
./lib/v4.11.0/android-arm64-v8a-release/verify_architecture.sh
```

### 手动验证
```bash
# 1. 检查库文件
ls -la lib/v4.11.0/android-arm64-v8a-release/lib/libopencv_*.a

# 2. 检查大小（ARM64库通常较大）
du -sh lib/v4.11.0/android-arm64-v8a-release/lib/

# 3. 检查配置文件
grep "OpenCV_ANDROID_ABI" lib/v4.11.0/android-arm64-v8a-release/OpenCVConfig.cmake
```

## 🧪 快速测试

### C++测试
```bash
# 编译测试程序
cd test
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21 ..
make
```

### Python测试（如果启用了Python支持）
```python
import cv2
print(f"OpenCV版本: {cv2.__version__}")
img = cv2.imread("test.jpg")
print(f"图像尺寸: {img.shape}")
```

## 📁 文件结构

### 编译输出
```
lib/v4.11.0/android-arm64-v8a-release/
├── lib/                           # 静态库文件
│   ├── libopencv_core.a          # 核心模块
│   ├── libopencv_imgproc.a        # 图像处理
│   ├── libopencv_imgcodecs.a      # 图像编解码
│   └── ...                        # 其他模块
├── include/opencv2/              # C++头文件
├── OpenCVConfig.cmake            # CMake配置文件
├── README.md                     # 详细说明
└── verify_architecture.sh        # 验证脚本
```

## 🔗 在项目中使用

### CMake项目
```cmake
# 设置OpenCV路径
set(OpenCV_DIR "/path/to/opencv/lib/v4.11.0/android-arm64-v8a-release")

# 查找OpenCV包
find_package(OpenCV REQUIRED)

# 链接库
target_link_libraries(your_target PRIVATE OpenCV)
```

### Android.mk项目
```makefile
LOCAL_STATIC_LIBRARIES += opencv_core opencv_imgproc opencv_highgui
```

## ⚠️ 常见问题

### 问题1: 编译失败
```bash
# 错误: Could not find toolchain file
export ANDROID_NDK_HOME="/correct/ndk/path"
```

### 问题2: ABI不匹配
```bash
# 确保CMake参数包含
-DANDROID_ABI=arm64-v8a
```

### 问题3: 链接错误
```cmake
# 确保OpenCV_DIR指向正确目录
set(OpenCV_DIR "/absolute/path/to/android-arm64-v8a-release")
```

## 📞 获取帮助

### 查看日志
```bash
# 详细编译输出
make VERBOSE=1

# CMake配置日志
cat CMakeCache.txt | grep -i android
```

### 检查文档
```bash
# 完整文档
cat Android-ARM64-Build-Guide.md

# 编译脚本说明
head -20 compiler/scripts/build-android-arm64-v8a-release-fixed.sh
```

## 🎯 性能提示

### 编译优化
- 使用SSD硬盘加速编译
- 增加并行编译线程: `make -j$(nproc)`
- 禁用不需要的模块

### 运行时优化
- 启用NEON SIMD指令
- 使用适当的图像格式
- 预分配内存缓冲区

---

**快速参考版本**: 1.0  
**适用**: OpenCV 4.11.0 + Android ARM64  
**最后更新**: 2026-05-04