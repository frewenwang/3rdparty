// native-lib.cpp
#include <jni.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <android/log.h>

#define LOG_TAG "OpenCV_JNI"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" {

/**
 * 图像灰度化 JNI 接口
 * @param env JNI 环境
 * @param instance Java 实例
 * @param inputPath 输入图像路径
 * @param outputPath 输出图像路径
 * @return 处理是否成功
 */
JNIEXPORT jboolean JNICALL
Java_com_example_opencv_NativeProcessor_convertToGray(
    JNIEnv *env,
    jobject instance,
    jstring inputPath,
    jstring outputPath) {

    const char *input_path = env->GetStringUTFChars(inputPath, 0);
    const char *output_path = env->GetStringUTFChars(outputPath, 0);

    try {
        // 读取图像
        cv::Mat image = cv::imread(input_path);
        if (image.empty()) {
            LOGE("无法读取图像: %s", input_path);
            return JNI_FALSE;
        }

        // 转换为灰度图
        cv::Mat grayImage;
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);

        // 保存结果
        bool result = cv::imwrite(output_path, grayImage);

        // 释放资源
        image.release();
        grayImage.release();

        env->ReleaseStringUTFChars(inputPath, input_path);
        env->ReleaseStringUTFChars(outputPath, output_path);

        return result ? JNI_TRUE : JNI_FALSE;

    } catch (const std::exception& e) {
        LOGE("处理异常: %s", e.what());
        env->ReleaseStringUTFChars(inputPath, input_path);
        env->ReleaseStringUTFChars(outputPath, output_path);
        return JNI_FALSE;
    }
}

/**
 * 图像模糊处理 JNI 接口
 * @param env JNI 环境
 * @param instance Java 实例
 * @param inputPath 输入图像路径
 * @param outputPath 输出图像路径
 * @param kernelSize 模糊核大小
 * @return 处理是否成功
 */
JNIEXPORT jboolean JNICALL
Java_com_example_opencv_NativeProcessor_applyBlur(
    JNIEnv *env,
    jobject instance,
    jstring inputPath,
    jstring outputPath,
    jint kernelSize) {

    const char *input_path = env->GetStringUTFChars(inputPath, 0);
    const char *output_path = env->GetStringUTFChars(outputPath, 0);

    try {
        cv::Mat image = cv::imread(input_path);
        if (image.empty()) {
            LOGE("无法读取图像: %s", input_path);
            return JNI_FALSE;
        }

        cv::Mat blurredImage;
        cv::GaussianBlur(image, blurredImage,
                        cv::Size(kernelSize, kernelSize), 0);

        bool result = cv::imwrite(output_path, blurredImage);

        image.release();
        blurredImage.release();

        env->ReleaseStringUTFChars(inputPath, input_path);
        env->ReleaseStringUTFChars(outputPath, output_path);

        return result ? JNI_TRUE : JNI_FALSE;

    } catch (const std::exception& e) {
        LOGE("处理异常: %s", e.what());
        env->ReleaseStringUTFChars(inputPath, input_path);
        env->ReleaseStringUTFChars(outputPath, output_path);
        return JNI_FALSE;
    }
}

/**
 * 边缘检测 JNI 接口
 * @param env JNI 环境
 * @param instance Java 实例
 * @param inputPath 输入图像路径
 * @param outputPath 输出图像路径
 * @param threshold1 低阈值
 * @param threshold2 高阈值
 * @return 处理是否成功
 */
JNIEXPORT jboolean JNICALL
Java_com_example_opencv_NativeProcessor_detectEdges(
    JNIEnv *env,
    jobject instance,
    jstring inputPath,
    jstring outputPath,
    jdouble threshold1,
    jdouble threshold2) {

    const char *input_path = env->GetStringUTFChars(inputPath, 0);
    const char *output_path = env->GetStringUTFChars(outputPath, 0);

    try {
        cv::Mat image = cv::imread(input_path, cv::IMREAD_GRAYSCALE);
        if (image.empty()) {
            LOGE("无法读取图像: %s", input_path);
            return JNI_FALSE;
        }

        cv::Mat edges;
        cv::Canny(image, edges, threshold1, threshold2);

        bool result = cv::imwrite(output_path, edges);

        image.release();
        edges.release();

        env->ReleaseStringUTFChars(inputPath, input_path);
        env->ReleaseStringUTFChars(outputPath, output_path);

        return result ? JNI_TRUE : JNI_FALSE;

    } catch (const std::exception& e) {
        LOGE("处理异常: %s", e.what());
        env->ReleaseStringUTFChars(inputPath, input_path);
        env->ReleaseStringUTFChars(outputPath, output_path);
        return JNI_FALSE;
    }
}

/**
 * 图像矩阵处理 JNI 接口（直接操作 Mat 对象）
 * @param env JNI 环境
 * @param instance Java 实例
 * @param matAddr Mat 对象地址
 * @param operation 操作类型 (1: 灰度化, 2: 模糊, 3: 边缘检测)
 */
JNIEXPORT void JNICALL
Java_com_example_opencv_NativeProcessor_processMat(
    JNIEnv *env,
    jobject instance,
    jlong matAddr,
    jint operation) {

    cv::Mat& image = *(cv::Mat*)matAddr;

    try {
        switch (operation) {
            case 1: // 灰度化
                if (image.channels() > 1) {
                    cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
                }
                break;

            case 2: // 高斯模糊
                cv::GaussianBlur(image, image, cv::Size(15, 15), 0);
                break;

            case 3: // 边缘检测
                if (image.channels() > 1) {
                    cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
                }
                cv::Canny(image, image, 50, 150);
                break;

            default:
                LOGE("未知操作类型: %d", operation);
                break;
        }

    } catch (const std::exception& e) {
        LOGE("矩阵处理异常: %s", e.what());
    }
}

/**
 * 人脸检测 JNI 接口
 * @param env JNI 环境
 * @param instance Java 实例
 * @param inputPath 输入图像路径
 * @param outputPath 输出图像路径
 * @param cascadePath 级联分类器路径
 * @return 检测到的人脸数量
 */
JNIEXPORT jint JNICALL
Java_com_example_opencv_NativeProcessor_detectFaces(
    JNIEnv *env,
    jobject instance,
    jstring inputPath,
    jstring outputPath,
    jstring cascadePath) {

    const char *input_path = env->GetStringUTFChars(inputPath, 0);
    const char *output_path = env->GetStringUTFChars(outputPath, 0);
    const char *cascade_path = env->GetStringUTFChars(cascadePath, 0);

    try {
        // 加载图像
        cv::Mat image = cv::imread(input_path);
        if (image.empty()) {
            LOGE("无法读取图像: %s", input_path);
            return 0;
        }

        // 加载级联分类器
        cv::CascadeClassifier faceCascade;
        if (!faceCascade.load(cascade_path)) {
            LOGE("无法加载级联分类器: %s", cascade_path);
            return 0;
        }

        // 转换为灰度图
        cv::Mat grayImage;
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(grayImage, grayImage);

        // 检测人脸
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(grayImage, faces, 1.1, 3, 0,
                                    cv::Size(30, 30));

        // 绘制检测结果
        for (size_t i = 0; i < faces.size(); i++) {
            cv::rectangle(image, faces[i], cv::Scalar(0, 255, 0), 2);

            // 添加标签
            std::string label = "Face " + std::to_string(i + 1);
            cv::Point labelPos(faces[i].x, faces[i].y - 10);
            cv::putText(image, label, labelPos, cv::FONT_HERSHEY_SIMPLEX, 0.7,
                       cv::Scalar(0, 255, 0), 2);
        }

        // 保存结果
        cv::imwrite(output_path, image);

        // 释放资源
        image.release();
        grayImage.release();

        env->ReleaseStringUTFChars(inputPath, input_path);
        env->ReleaseStringUTFChars(outputPath, output_path);
        env->ReleaseStringUTFChars(cascadePath, cascade_path);

        return (jint)faces.size();

    } catch (const std::exception& e) {
        LOGE("人脸检测异常: %s", e.what());
        env->ReleaseStringUTFChars(inputPath, input_path);
        env->ReleaseStringUTFChars(outputPath, output_path);
        env->ReleaseStringUTFChars(cascadePath, cascade_path);
        return 0;
    }
}

/**
 * 图像特征检测 JNI 接口
 * @param env JNI 环境
 * @param instance Java 实例
 * @param inputPath 输入图像路径
 * @param outputPath 输出图像路径
 * @param method 特征检测方法 (1: SIFT, 2: SURF, 3: ORB)
 * @return 检测到的特征点数量
 */
JNIEXPORT jint JNICALL
Java_com_example_opencv_NativeProcessor_detectFeatures(
    JNIEnv *env,
    jobject instance,
    jstring inputPath,
    jstring outputPath,
    jint method) {

    const char *input_path = env->GetStringUTFChars(inputPath, 0);
    const char *output_path = env->GetStringUTFChars(outputPath, 0);

    try {
        cv::Mat image = cv::imread(input_path);
        if (image.empty()) {
            LOGE("无法读取图像: %s", input_path);
            return 0;
        }

        cv::Mat grayImage;
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);

        std::vector<cv::KeyPoint> keypoints;
        cv::Ptr<cv::Feature2D> detector;

        // 选择特征检测器
        switch (method) {
            case 1:
                // SIFT
                detector = cv::SIFT::create();
                break;
            case 2:
                // SURF
                detector = cv::xfeatures2d::SURF::create();
                break;
            case 3:
            default:
                // ORB
                detector = cv::ORB::create();
                break;
        }

        // 检测特征点
        detector->detect(grayImage, keypoints);

        // 绘制特征点
        cv::Mat resultImage;
        cv::drawKeypoints(image, keypoints, resultImage,
                         cv::Scalar(0, 255, 0),
                         cv::DrawMatchesFlags::DEFAULT);

        // 保存结果
        cv::imwrite(output_path, resultImage);

        // 释放资源
        image.release();
        grayImage.release();
        resultImage.release();

        env->ReleaseStringUTFChars(inputPath, input_path);
        env->ReleaseStringUTFChars(outputPath, output_path);

        return (jint)keypoints.size();

    } catch (const std::exception& e) {
        LOGE("特征检测异常: %s", e.what());
        env->ReleaseStringUTFChars(inputPath, input_path);
        env->ReleaseStringUTFChars(outputPath, output_path);
        return 0;
    }
}

/**
 * 性能测试 JNI 接口
 * @param env JNI 环境
 * @param instance Java 实例
 * @param inputPath 输入图像路径
 * @param iterations 迭代次数
 * @return 平均处理时间（毫秒）
 */
JNIEXPORT jdouble JNICALL
Java_com_example_opencv_NativeProcessor_performanceTest(
    JNIEnv *env,
    jobject instance,
    jstring inputPath,
    jint iterations) {

    const char *input_path = env->GetStringUTFChars(inputPath, 0);

    try {
        cv::Mat image = cv::imread(input_path);
        if (image.empty()) {
            LOGE("无法读取图像: %s", input_path);
            return -1.0;
        }

        // 预热
        cv::Mat temp;
        cv::GaussianBlur(image, temp, cv::Size(15, 15), 0);

        // 性能测试
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; i++) {
            cv::GaussianBlur(image, temp, cv::Size(15, 15), 0);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        double avgTime = (double)duration.count() / iterations / 1000.0; // 转换为毫秒

        image.release();
        temp.release();

        env->ReleaseStringUTFChars(inputPath, input_path);

        return avgTime;

    } catch (const std::exception& e) {
        LOGE("性能测试异常: %s", e.what());
        env->ReleaseStringUTFChars(inputPath, input_path);
        return -1.0;
    }
}

} // extern "C"