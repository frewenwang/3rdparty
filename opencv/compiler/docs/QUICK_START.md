# OpenCV Android ARM64 快速开始指南

## 🚀 5分钟快速开始

### 步骤 1: 环境设置 (30秒)

```bash
# 设置环境变量
export ANDROID_NDK_HOME=/home/frewen/snap/Android/ndk/android-ndk-r26d
export ANDROID_SDK_ROOT=/home/frewen/Android/Sdk
export ANDROID_HOME=/home/frewen/Android/Sdk
```

### 步骤 2: 执行构建 (2分钟)

```bash
# 进入项目目录
cd /home/frewen/01.WorkSpace/AuraKaleidos/FantasyCXX/3rdparty/opencv/compiler

# 执行构建（如果尚未构建）
./scripts/build-android-armv8a-release.sh
```

### 步骤 3: 验证构建 (30秒)

```bash
# 检查生成的库
ls build/android-arm64-v8a-release/lib/armeabi-v7a/

# 预期输出: 16个 .a 静态库文件
libopencv_core.a      libopencv_imgproc.a     libopencv_dnn.a
...
```

### 步骤 4: 运行测试 (1分钟)

```bash
# 运行核心功能测试
cd build/android-arm64-v8a-release/bin
./opencv_test_core --gtest_filter="*Simple*"
```

### 步骤 5: 使用示例代码 (1分钟)

```java
// 在您的 Android 项目中
public class MyOpenCVApp {
    static {
        System.loadLibrary("opencv_java4");
    }
    
    public void processImage() {
        // 使用 OpenCV 功能
        Mat image = Imgcodecs.imread("/sdcard/input.jpg");
        Mat gray = new Mat();
        Imgproc.cvtColor(image, gray, Imgproc.COLOR_BGR2GRAY);
        Imgcodecs.imwrite("/sdcard/output.jpg", gray);
    }
}
```

## 📋 构建状态检查清单

### ✅ 环境检查
- [ ] ANDROID_NDK_HOME 已设置
- [ ] ANDROID_SDK_ROOT 已设置
- [ ] 磁盘空间 > 20GB
- [ ] 内存 > 8GB

### ✅ 构建检查
- [ ] 构建脚本执行成功
- [ ] 生成 16 个静态库文件
- [ ] 生成测试程序
- [ ] 无编译错误

### ✅ 测试检查
- [ ] 核心测试通过
- [ ] 图像处理测试通过
- [ ] 性能在可接受范围内

## 🎯 常用命令速查

### 构建命令
```bash
# 完整构建
./scripts/build-android-armv8a-release.sh

# 清理构建
rm -rf build/android-arm64-v8a-release

# 重新构建
make clean && make -j22
```

### 测试命令
```bash
# 核心功能测试
./opencv_test_core

# 图像处理测试
./opencv_test_imgproc

# 性能测试
./opencv_perf_core

# 特定测试
./opencv_test_core --gtest_filter="Mat_*"
```

### 调试命令
```bash
# 检查库文件
ls -la build/android-arm64-v8a-release/lib/armeabi-v7a/

# 检查测试程序
ls -la build/android-arm64-v8a-release/bin/

# 查看构建日志
cat build/android-arm64-v8a-release/CMakeCache.txt
```

## 🔧 常见问题快速解决

### 问题 1: "ANDROID_NDK_HOME is not set"
```bash
# 解决方案
export ANDROID_NDK_HOME=/correct/ndk/path
```

### 问题 2: 构建失败
```bash
# 检查日志
cat build/android-arm64-v8a-release/CMakeCache.txt

# 清理重试
rm -rf build/android-arm64-v8a-release
./scripts/build-android-armv8a-release.sh
```

### 问题 3: 测试失败
```bash
# 运行简单测试
./opencv_test_core --gtest_filter="*Simple*"

# 查看详细输出
./opencv_test_core --gtest_repeat=1 --gtest_break_on_failure
```

## 📚 文档导航

### 需要详细了解？
- **构建原理**: 阅读 [OpenCV_Build_Documentation_zh.md](OpenCV_Build_Documentation_zh.md)
- **编译配置**: 阅读 [compilation_guide.md](compilation_guide.md)
- **测试方法**: 阅读 [testing_guide.md](testing_guide.md)

### 需要示例代码？
- **基础处理**: 查看 [BasicImageProcessor.java](examples/BasicImageProcessor.java)
- **人脸检测**: 查看 [FaceDetection.java](examples/FaceDetection.java)
- **JNI 接口**: 查看 [native-lib.cpp](examples/native-lib.cpp)

### 需要集成帮助？
- **Android 集成**: 阅读 [examples/README.md](examples/README.md)
- **CMake 配置**: 查看 [CMakeLists.txt](examples/CMakeLists.txt)

## 📊 构建成果速览

### 生成的库文件
- **16 个核心模块**
- **总计 418 MB**
- **支持 Java 绑定**
- **NEON 优化**

### 主要功能
- ✅ 图像处理 (灰度化、模糊、边缘检测等)
- ✅ 特征检测 (SIFT、SURF、ORB)
- ✅ 目标检测 (人脸、眼睛等)
- ✅ 深度学习 (DNN 模块)
- ✅ 视频处理
- ✅ 机器学习

## 🎓 学习路径建议

### 第1天: 基础入门
1. 阅读本快速开始指南
2. 成功执行构建
3. 运行基础测试
4. 尝试 BasicImageProcessor.java

### 第2天: 进阶学习
1. 阅读 compilation_guide.md
2. 学习 testing_guide.md
3. 尝试 FaceDetection.java
4. 运行性能测试

### 第3天: 高级应用
1. 阅读 OpenCV_Build_Documentation_zh.md
2. 学习 native-lib.cpp
3. 开发自定义功能
4. 集成到 Android 项目

## 🏆 成功标志

当您完成以下任务时，表示已经成功掌握：

- [ ] 成功构建了 OpenCV Android ARM64 库
- [ ] 运行了所有测试程序
- [ ] 使用示例代码处理了图像
- [ ] 将 OpenCV 集成到了 Android 项目
- [ ] 开发了自定义的图像处理功能

## 🚨 紧急联系

### 如果遇到问题：
1. 查看相关文档章节
2. 检查构建日志
3. 运行测试程序诊断
4. 参考示例代码

### 常用调试信息：
```bash
# 查看系统信息
echo $ANDROID_NDK_HOME
echo $ANDROID_SDK_ROOT

# 查看构建信息
ls -la build/android-arm64-v8a-release/

# 查看测试结果
cat build/android-arm64-v8a-release/bin/test_*.xml
```

---

**恭喜！您已经掌握了 OpenCV Android ARM64 构建工程的基本使用！** 🎉

**下一步建议：**
1. 深入学习详细文档
2. 尝试更多示例代码
3. 开发您的 OpenCV 应用
4. 分享给其他开发者

**祝您开发顺利！** 🚀