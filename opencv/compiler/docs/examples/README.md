# OpenCV Android ARM64 示例代码

## 📁 示例代码结构

```
examples/
├── BasicImageProcessor.java    # 基础图像处理示例
├── FaceDetection.java          # 人脸检测示例
├── native-lib.cpp             # C++ JNI 接口示例
├── CMakeLists.txt            # CMake 构建配置
└── README.md                 # 本文件
```

## 🚀 快速开始

### 1. Java 示例

#### 1.1 基础图像处理 (BasicImageProcessor.java)

这个示例展示了 OpenCV 的基本图像处理功能：

```java
// 初始化 OpenCV
static {
    System.loadLibrary("opencv_java4");
}

// 灰度化
ImageProcessor.convertToGray("/sdcard/input.jpg", "/sdcard/gray.jpg");

// 高斯模糊
ImageProcessor.applyGaussianBlur("/sdcard/input.jpg", "/sdcard/blur.jpg", 15);

// 边缘检测
ImageProcessor.detectEdges("/sdcard/input.jpg", "/sdcard/edges.jpg", 50, 150);
```

**支持的功能：**
- ✅ 图像灰度化
- ✅ 高斯模糊
- ✅ 边缘检测 (Canny)
- ✅ 图像缩放
- ✅ 图像旋转
- ✅ 直方图均衡化
- ✅ 阈值处理
- ✅ 形态学操作
- ✅ 轮廓检测

#### 1.2 人脸检测 (FaceDetection.java)

这个示例展示了人脸检测和特征识别：

```java
// 初始化检测器
FaceDetection detector = new FaceDetection();
detector.initDetectors(
    "/sdcard/haarcascade_frontalface_default.xml",
    "/sdcard/haarcascade_eye.xml"
);

// 检测人脸
int faceCount = detector.detectFaces("/sdcard/input.jpg", "/sdcard/output.jpg");
System.out.println("检测到 " + faceCount + " 张人脸");
```

**支持的功能：**
- ✅ 基础人脸检测
- ✅ 高级人脸检测（带参数调节）
- ✅ 眼睛检测
- ✅ 嘴巴检测
- ✅ 批量图像处理
- ✅ 检测统计

### 2. C++ JNI 示例

#### 2.1 原生接口 (native-lib.cpp)

这个示例展示了如何通过 JNI 调用 C++ OpenCV 功能：

```cpp
// 图像灰度化
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_opencv_NativeProcessor_convertToGray(
    JNIEnv *env, jobject instance,
    jstring inputPath, jstring outputPath) {
    
    const char *input_path = env->GetStringUTFChars(inputPath, 0);
    const char *output_path = env->GetStringUTFChars(outputPath, 0);
    
    cv::Mat image = cv::imread(input_path);
    cv::Mat grayImage;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    bool result = cv::imwrite(output_path, grayImage);
    
    // 释放资源...
    return result ? JNI_TRUE : JNI_FALSE;
}
```

**支持的功能：**
- ✅ 图像灰度化
- ✅ 高斯模糊
- ✅ 边缘检测
- ✅ 直接矩阵操作
- ✅ 人脸检测
- ✅ 特征检测 (SIFT/SURF/ORB)
- ✅ 性能测试

## 🔧 编译配置

### CMakeLists.txt

```cmake
# 查找 OpenCV 包
find_package(OpenCV REQUIRED)

# 添加原生库
add_library(native-lib SHARED
            src/main/cpp/native-lib.cpp)

# 链接 OpenCV 库
target_link_libraries(native-lib
                     ${OpenCV_LIBS})

# ARM NEON 优化
target_compile_options(native-lib PRIVATE
                      -march=armv7-a
                      -mfpu=neon)
```

## 📱 Android 集成指南

### 步骤 1: 添加 OpenCV 依赖

**build.gradle:**
```gradle
android {
    defaultConfig {
        ndk {
            abiFilters 'armeabi-v7a'
        }
    }
    
    sourceSets {
        main {
            jniLibs.srcDirs = ['src/main/jniLibs']
        }
    }
}

dependencies {
    implementation project(':opencv')
}
```

### 步骤 2: 配置 JNI 接口

**MainActivity.java:**
```java
public class MainActivity extends AppCompatActivity {
    
    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            if (status == LoaderCallbackInterface.SUCCESS) {
                Log.i("OpenCV", "OpenCV loaded successfully");
                // OpenCV 加载成功，可以使用相关功能
            }
        }
    };
    
    @Override
    protected void onResume() {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_4_11_0, 
                                 this, mLoaderCallback);
        }
    }
}
```

### 步骤 3: 使用图像处理功能

```java
public class ImageProcessor {
    
    // 声明原生方法
    public native boolean convertToGray(String inputPath, String outputPath);
    public native boolean applyBlur(String inputPath, String outputPath, int kernelSize);
    public native int detectFaces(String inputPath, String outputPath, String cascadePath);
    
    static {
        System.loadLibrary("native-lib");
    }
}
```

## 🎯 性能优化建议

### 1. 内存管理

```java
// 及时释放 Mat 对象
Mat image = Imgcodecs.imread(path);
try {
    // 处理图像...
} finally {
    image.release();
}

// 复用 Mat 对象
private Mat reusableMat = new Mat();
```

### 2. 图像处理优化

```java
// 使用合适的图像尺寸
Mat resized = new Mat();
Imgproc.resize(largeImage, resized, new Size(640, 480));

// 选择适当的算法参数
Imgproc.GaussianBlur(image, result, new Size(15, 15), 0);
```

### 3. JNI 调用优化

```cpp
// 减少 JNI 调用次数
// 不好的做法：
for (int i = 0; i < 100; i++) {
    processImageJNI(env, imagePath);  // 100 次 JNI 调用
}

// 好的做法：
processBatchImagesJNI(env, imagePaths);  // 1 次 JNI 调用处理所有图像
```

## 📊 性能基准

| 操作 | 图像尺寸 | 处理时间 (ms) | 推荐使用场景 |
|------|----------|---------------|-------------|
| 灰度转换 | 1920x1080 | 15-25 | 实时处理 |
| 高斯模糊 | 1920x1080 | 45-65 | 离线处理 |
| 边缘检测 | 1920x1080 | 35-50 | 实时处理 |
| 人脸检测 | 640x480 | 120-180 | 离线处理 |

## 🚨 常见问题

### Q1: 如何添加 OpenCV 到 Android 项目？

**解决方案：**
1. 将生成的 `.so` 文件复制到 `app/src/main/jniLibs/armeabi-v7a/`
2. 在 build.gradle 中添加 OpenCV 依赖
3. 配置 CMakeLists.txt

### Q2: JNI 调用时出现崩溃怎么办？

**解决方案：**
1. 检查异常处理
2. 确保正确释放资源
3. 验证输入参数

```cpp
try {
    // JNI 代码
} catch (const std::exception& e) {
    LOGE("JNI Exception: %s", e.what());
    return JNI_FALSE;
}
```

### Q3: 如何优化处理速度？

**解决方案：**
1. 使用 NEON 优化
2. 减少图像尺寸
3. 复用对象
4. 批量处理

## 📚 更多资源

- [OpenCV 官方文档](https://docs.opencv.org/)
- [Android NDK 指南](https://developer.android.com/ndk/guides)
- [JNI 编程指南](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/)

---

**开始使用这些示例代码来开发您的 OpenCV Android 应用吧！** 🎉