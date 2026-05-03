package com.example.opencvandroid;

import org.opencv.core.*;
import org.opencv.imgcodecs.Imgcodecs;
import org.opencv.imgproc.Imgproc;
import org.opencv.objdetect.CascadeClassifier;
import org.opencv.objdetect.Objdetect;

public class FaceDetection {

    private CascadeClassifier faceCascade;
    private CascadeClassifier eyesCascade;

    /**
     * 初始化人脸检测器
     * @param faceCascadePath 人脸检测分类器路径
     * @param eyesCascadePath 眼睛检测分类器路径
     */
    public void initDetectors(String faceCascadePath, String eyesCascadePath) {
        faceCascade = new CascadeClassifier(faceCascadePath);
        eyesCascade = new CascadeClassifier(eyesCascadePath);

        if (faceCascade.empty()) {
            throw new RuntimeException("无法加载人脸检测分类器");
        }
    }

    /**
     * 检测图像中的人脸
     * @param inputPath 输入图像路径
     * @param outputPath 输出图像路径
     * @return 检测到的人脸数量
     */
    public int detectFaces(String inputPath, String outputPath) {
        Mat image = Imgcodecs.imread(inputPath);
        if (image.empty()) {
            return 0;
        }

        // 转换为灰度图
        Mat grayImage = new Mat();
        Imgproc.cvtColor(image, grayImage, Imgproc.COLOR_BGR2GRAY);
        Imgproc.equalizeHist(grayImage, grayImage);

        // 检测人脸
        MatOfRect faces = new MatOfRect();
        faceCascade.detectMultiScale(grayImage, faces, 1.1, 3, 0,
            new Size(30, 30));

        int faceCount = 0;
        for (Rect face : faces.toArray()) {
            // 绘制人脸矩形
            Imgproc.rectangle(image, face, new Scalar(0, 255, 0), 3);

            // 在人脸区域内检测眼睛
            Mat faceROI = grayImage.submat(face);
            MatOfRect eyes = new MatOfRect();
            eyesCascade.detectMultiScale(faceROI, eyes);

            // 绘制眼睛矩形
            for (Rect eye : eyes.toArray()) {
                Point eyeCenter = new Point(face.x + eye.x + eye.width/2,
                                          face.y + eye.y + eye.height/2);
                int radius = (int) (eye.width + eye.height) * 0.25;
                Imgproc.circle(image, eyeCenter, radius, new Scalar(255, 0, 0), 3);
            }

            faceCount++;
        }

        // 保存结果
        Imgcodecs.imwrite(outputPath, image);

        // 释放资源
        image.release();
        grayImage.release();
        faces.release();

        return faceCount;
    }

    /**
     * 高级人脸检测（带参数调节）
     * @param inputPath 输入图像路径
     * @param outputPath 输出图像路径
     * @param scaleFactor 尺度缩放因子
     * @param minNeighbors 最小邻居数
     * @param minSize 最小检测窗口大小
     * @param maxSize 最大检测窗口大小
     * @return 检测到的人脸信息
     */
    public java.util.List<FaceInfo> detectFacesAdvanced(String inputPath, String outputPath,
                                                       double scaleFactor, int minNeighbors,
                                                       Size minSize, Size maxSize) {
        Mat image = Imgcodecs.imread(inputPath);
        if (image.empty()) {
            return new java.util.ArrayList<>();
        }

        Mat grayImage = new Mat();
        Imgproc.cvtColor(image, grayImage, Imgproc.COLOR_BGR2GRAY);
        Imgproc.equalizeHist(grayImage, grayImage);

        // 高级参数的人脸检测
        MatOfRect faces = new MatOfRect();
        faceCascade.detectMultiScale(grayImage, faces, scaleFactor, minNeighbors,
            Objdetect.CASCADE_SCALE_IMAGE, minSize, maxSize);

        java.util.List<FaceInfo> faceInfoList = new java.util.ArrayList<>();

        for (Rect face : faces.toArray()) {
            FaceInfo info = new FaceInfo();
            info.faceRect = face;
            info.confidence = 1.0; // 默认置信度

            // 绘制人脸矩形
            Imgproc.rectangle(image, face, new Scalar(0, 255, 0), 2);

            // 添加标签
            String label = String.format("Face %d", faceInfoList.size() + 1);
            Point labelPos = new Point(face.x, face.y - 10);
            Imgproc.putText(image, label, labelPos, Imgproc.FONT_HERSHEY_SIMPLEX, 0.7,
                           new Scalar(0, 255, 0), 2);

            // 检测眼睛
            Mat faceROI = grayImage.submat(face);
            MatOfRect eyes = new MatOfRect();
            eyesCascade.detectMultiScale(faceROI, eyes, 1.1, 2, 0,
                new Size(20, 20), new Size(face.width/2, face.height/2));

            info.eyeCount = eyes.toArray().length;

            for (Rect eye : eyes.toArray()) {
                Point eyeCenter = new Point(face.x + eye.x + eye.width/2,
                                          face.y + eye.y + eye.height/2);
                int radius = (int) (eye.width + eye.height) * 0.25;
                Imgproc.circle(image, eyeCenter, radius, new Scalar(255, 0, 0), 2);
            }

            // 检测嘴巴（简化版）
            Rect mouthRegion = new Rect(
                face.x + face.width/4,
                face.y + face.height*2/3,
                face.width/2,
                face.height/3
            );

            Mat mouthROI = grayImage.submat(mouthRegion);
            double mouthMean = Core.mean(mouthROI).val[0];
            info.brightness = mouthMean;

            if (mouthMean < 100) { // 简单的嘴巴检测
                Imgproc.rectangle(image, mouthRegion, new Scalar(0, 0, 255), 2);
                info.hasMouth = true;
            }

            faceInfoList.add(info);
        }

        Imgcodecs.imwrite(outputPath, image);

        // 释放资源
        image.release();
        grayImage.release();
        faces.release();

        return faceInfoList;
    }

    /**
     * 批量处理多张图像
     * @param imagePaths 图像路径列表
     * @param outputDir 输出目录
     * @return 处理结果统计
     */
    public DetectionStatistics batchProcess(java.util.List<String> imagePaths, String outputDir) {
        DetectionStatistics stats = new DetectionStatistics();

        for (String imagePath : imagePaths) {
            try {
                String fileName = new java.io.File(imagePath).getName();
                String outputPath = outputDir + "/detected_" + fileName;

                int faceCount = detectFaces(imagePath, outputPath);

                stats.totalImages++;
                stats.totalFaces += faceCount;

                if (faceCount > 0) {
                    stats.imagesWithFaces++;
                }

                System.out.println("处理 " + fileName + ": 检测到 " + faceCount + " 张人脸");

            } catch (Exception e) {
                stats.failedImages++;
                System.err.println("处理失败 " + imagePath + ": " + e.getMessage());
            }
        }

        return stats;
    }

    /**
     * 人脸信息类
     */
    public static class FaceInfo {
        public Rect faceRect;
        public double confidence;
        public int eyeCount;
        public boolean hasMouth;
        public double brightness;

        @Override
        public String toString() {
            return String.format("Face{Rect=%s, Confidence=%.2f, Eyes=%d, Mouth=%s, Brightness=%.1f}",
                faceRect, confidence, eyeCount, hasMouth, brightness);
        }
    }

    /**
     * 检测统计类
     */
    public static class DetectionStatistics {
        public int totalImages = 0;
        public int totalFaces = 0;
        public int imagesWithFaces = 0;
        public int failedImages = 0;

        public double getAverageFacesPerImage() {
            return totalImages > 0 ? (double) totalFaces / totalImages : 0.0;
        }

        public double getSuccessRate() {
            return totalImages > 0 ? (double) (totalImages - failedImages) / totalImages * 100 : 0.0;
        }

        @Override
        public String toString() {
            return String.format(
                "DetectionStatistics{Total=%d, Faces=%d, WithFaces=%d, Failed=%d, AvgFaces=%.2f, SuccessRate=%.1f%%}",
                totalImages, totalFaces, imagesWithFaces, failedImages,
                getAverageFacesPerImage(), getSuccessRate());
        }
    }

    /**
     * 使用示例
     */
    public static class Example {
        public static void main(String[] args) {
            FaceDetection detector = new FaceDetection();

            // 初始化检测器
            detector.initDetectors(
                "/sdcard/haarcascade_frontalface_default.xml",
                "/sdcard/haarcascade_eye.xml"
            );

            // 单张图像检测
            int faceCount = detector.detectFaces("/sdcard/input.jpg", "/sdcard/output.jpg");
            System.out.println("检测到 " + faceCount + " 张人脸");

            // 高级检测
            java.util.List<FaceInfo> faces = detector.detectFacesAdvanced(
                "/sdcard/input.jpg", "/sdcard/output_advanced.jpg",
                1.1, 3, new Size(30, 30), new Size(300, 300)
            );

            for (FaceInfo face : faces) {
                System.out.println(face);
            }

            // 批量处理
            java.util.List<String> imageList = java.util.Arrays.asList(
                "/sdcard/image1.jpg",
                "/sdcard/image2.jpg",
                "/sdcard/image3.jpg"
            );

            DetectionStatistics stats = detector.batchProcess(imageList, "/sdcard/output");
            System.out.println(stats);
        }
    }
}