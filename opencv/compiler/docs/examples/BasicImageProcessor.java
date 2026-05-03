package com.example.opencvandroid;

import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.CvType;
import org.opencv.core.Scalar;
import org.opencv.imgcodecs.Imgcodecs;
import org.opencv.imgproc.Imgproc;

public class BasicImageProcessor {

    static {
        // 加载 OpenCV 库
        if (!OpenCVLoader.initDebug()) {
            throw new RuntimeException("无法加载 OpenCV 库");
        }
    }

    /**
     * 基础图像处理示例
     */
    public static class ImageProcessor {

        /**
         * 图像灰度化
         * @param inputPath 输入图像路径
         * @param outputPath 输出图像路径
         * @return 处理是否成功
         */
        public static boolean convertToGray(String inputPath, String outputPath) {
            try {
                // 读取图像
                Mat image = Imgcodecs.imread(inputPath);
                if (image.empty()) {
                    System.err.println("无法读取图像: " + inputPath);
                    return false;
                }

                // 转换为灰度图
                Mat grayImage = new Mat();
                Imgproc.cvtColor(image, grayImage, Imgproc.COLOR_BGR2GRAY);

                // 保存结果
                boolean result = Imgcodecs.imwrite(outputPath, grayImage);

                // 释放资源
                image.release();
                grayImage.release();

                return result;

            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        /**
         * 图像模糊处理
         * @param inputPath 输入图像路径
         * @param outputPath 输出图像路径
         * @param kernelSize 模糊核大小
         * @return 处理是否成功
         */
        public static boolean applyGaussianBlur(String inputPath, String outputPath, int kernelSize) {
            try {
                Mat image = Imgcodecs.imread(inputPath);
                if (image.empty()) {
                    return false;
                }

                Mat blurredImage = new Mat();
                Imgproc.GaussianBlur(image, blurredImage,
                    new org.opencv.core.Size(kernelSize, kernelSize), 0);

                boolean result = Imgcodecs.imwrite(outputPath, blurredImage);

                image.release();
                blurredImage.release();

                return result;

            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        /**
         * 边缘检测
         * @param inputPath 输入图像路径
         * @param outputPath 输出图像路径
         * @param threshold1 低阈值
         * @param threshold2 高阈值
         * @return 处理是否成功
         */
        public static boolean detectEdges(String inputPath, String outputPath,
                                        double threshold1, double threshold2) {
            try {
                Mat image = Imgcodecs.imread(inputPath, Imgcodecs.IMREAD_GRAYSCALE);
                if (image.empty()) {
                    return false;
                }

                Mat edges = new Mat();
                Imgproc.Canny(image, edges, threshold1, threshold2);

                boolean result = Imgcodecs.imwrite(outputPath, edges);

                image.release();
                edges.release();

                return result;

            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        /**
         * 图像缩放
         * @param inputPath 输入图像路径
         * @param outputPath 输出图像路径
         * @param scaleX X轴缩放比例
         * @param scaleY Y轴缩放比例
         * @return 处理是否成功
         */
        public static boolean resizeImage(String inputPath, String outputPath,
                                        double scaleX, double scaleY) {
            try {
                Mat image = Imgcodecs.imread(inputPath);
                if (image.empty()) {
                    return false;
                }

                Mat resizedImage = new Mat();
                org.opencv.core.Size newSize = new org.opencv.core.Size(
                    (int)(image.width() * scaleX),
                    (int)(image.height() * scaleY)
                );
                Imgproc.resize(image, resizedImage, newSize);

                boolean result = Imgcodecs.imwrite(outputPath, resizedImage);

                image.release();
                resizedImage.release();

                return result;

            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        /**
         * 图像旋转
         * @param inputPath 输入图像路径
         * @param outputPath 输出图像路径
         * @param angle 旋转角度（度）
         * @return 处理是否成功
         */
        public static boolean rotateImage(String inputPath, String outputPath, double angle) {
            try {
                Mat image = Imgcodecs.imread(inputPath);
                if (image.empty()) {
                    return false;
                }

                // 计算图像中心点
                org.opencv.core.Point center = new org.opencv.core.Point(
                    image.cols() / 2.0, image.rows() / 2.0
                );

                // 获取旋转矩阵
                Mat rotationMatrix = Imgproc.getRotationMatrix2D(center, angle, 1.0);

                Mat rotatedImage = new Mat();
                Imgproc.warpAffine(image, rotatedImage, rotationMatrix, image.size());

                boolean result = Imgcodecs.imwrite(outputPath, rotatedImage);

                image.release();
                rotationMatrix.release();
                rotatedImage.release();

                return result;

            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        /**
         * 图像直方图均衡化
         * @param inputPath 输入图像路径
         * @param outputPath 输出图像路径
         * @return 处理是否成功
         */
        public static boolean equalizeHistogram(String inputPath, String outputPath) {
            try {
                Mat image = Imgcodecs.imread(inputPath, Imgcodecs.IMREAD_GRAYSCALE);
                if (image.empty()) {
                    return false;
                }

                Mat equalizedImage = new Mat();
                Imgproc.equalizeHist(image, equalizedImage);

                boolean result = Imgcodecs.imwrite(outputPath, equalizedImage);

                image.release();
                equalizedImage.release();

                return result;

            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        /**
         * 图像阈值处理
         * @param inputPath 输入图像路径
         * @param outputPath 输出图像路径
         * @param threshold 阈值
         * @param maxValue 最大值
         * @param thresholdType 阈值类型
         * @return 处理是否成功
         */
        public static boolean applyThreshold(String inputPath, String outputPath,
                                           double threshold, double maxValue, int thresholdType) {
            try {
                Mat image = Imgcodecs.imread(inputPath, Imgcodecs.IMREAD_GRAYSCALE);
                if (image.empty()) {
                    return false;
                }

                Mat thresholdedImage = new Mat();
                Imgproc.threshold(image, thresholdedImage, threshold, maxValue, thresholdType);

                boolean result = Imgcodecs.imwrite(outputPath, thresholdedImage);

                image.release();
                thresholdedImage.release();

                return result;

            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        /**
         * 形态学操作
         * @param inputPath 输入图像路径
         * @param outputPath 输出图像路径
         * @param operation 形态学操作类型
         * @param kernelSize 核大小
         * @return 处理是否成功
         */
        public static boolean morphologicalOperation(String inputPath, String outputPath,
                                                   int operation, int kernelSize) {
            try {
                Mat image = Imgcodecs.imread(inputPath, Imgcodecs.IMREAD_GRAYSCALE);
                if (image.empty()) {
                    return false;
                }

                Mat kernel = Imgproc.getStructuringElement(
                    Imgproc.MORPH_RECT,
                    new org.opencv.core.Size(kernelSize, kernelSize)
                );

                Mat resultImage = new Mat();
                Imgproc.morphologyEx(image, resultImage, operation, kernel);

                boolean result = Imgcodecs.imwrite(outputPath, resultImage);

                image.release();
                kernel.release();
                resultImage.release();

                return result;

            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        /**
         * 轮廓检测
         * @param inputPath 输入图像路径
         * @param outputPath 输出图像路径
         * @return 检测到的轮廓数量
         */
        public static int findContours(String inputPath, String outputPath) {
            try {
                Mat image = Imgcodecs.imread(inputPath, Imgcodecs.IMREAD_GRAYSCALE);
                if (image.empty()) {
                    return 0;
                }

                // 二值化
                Mat binaryImage = new Mat();
                Imgproc.threshold(image, binaryImage, 127, 255, Imgproc.THRESH_BINARY);

                // 查找轮廓
                java.util.List<org.opencv.core.MatOfPoint> contours = new java.util.ArrayList<>();
                Mat hierarchy = new Mat();
                Imgproc.findContours(binaryImage, contours, hierarchy,
                                   Imgproc.RETR_EXTERNAL, Imgproc.CHAIN_APPROX_SIMPLE);

                // 绘制轮廓
                Mat contourImage = Mat.zeros(image.size(), CvType.CV_8UC3);
                for (int i = 0; i < contours.size(); i++) {
                    org.opencv.core.Scalar color = new org.opencv.core.Scalar(0, 255, 0);
                    Imgproc.drawContours(contourImage, contours, i, color, 2);
                }

                Imgcodecs.imwrite(outputPath, contourImage);

                // 释放资源
                image.release();
                binaryImage.release();
                hierarchy.release();
                contourImage.release();

                for (org.opencv.core.MatOfPoint contour : contours) {
                    contour.release();
                }

                return contours.size();

            } catch (Exception e) {
                e.printStackTrace();
                return 0;
            }
        }
    }

    /**
     * 使用示例
     */
    public static class Example {
        public static void main(String[] args) {
            // 图像处理示例
            String inputPath = "/sdcard/input.jpg";
            String outputPath = "/sdcard/output.jpg";

            // 灰度化
            ImageProcessor.convertToGray(inputPath, "/sdcard/gray.jpg");

            // 高斯模糊
            ImageProcessor.applyGaussianBlur(inputPath, "/sdcard/blur.jpg", 15);

            // 边缘检测
            ImageProcessor.detectEdges(inputPath, "/sdcard/edges.jpg", 50, 150);

            // 图像缩放
            ImageProcessor.resizeImage(inputPath, "/sdcard/resized.jpg", 0.5, 0.5);

            // 图像旋转
            ImageProcessor.rotateImage(inputPath, "/sdcard/rotated.jpg", 45);

            // 直方图均衡化
            ImageProcessor.equalizeHistogram(inputPath, "/sdcard/equalized.jpg");

            // 阈值处理
            ImageProcessor.applyThreshold(inputPath, "/sdcard/threshold.jpg",
                                        127, 255, Imgproc.THRESH_BINARY);

            // 形态学操作 - 膨胀
            ImageProcessor.morphologicalOperation(inputPath, "/sdcard/dilate.jpg",
                                                Imgproc.MORPH_DILATE, 5);

            // 形态学操作 - 腐蚀
            ImageProcessor.morphologicalOperation(inputPath, "/sdcard/erode.jpg",
                                                Imgproc.MORPH_ERODE, 5);

            // 轮廓检测
            int contourCount = ImageProcessor.findContours(inputPath, "/sdcard/contours.jpg");
            System.out.println("检测到 " + contourCount + " 个轮廓");
        }
    }
}