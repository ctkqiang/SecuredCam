/**
 * @brief 主程序入口，实现人脸检测、识别和考勤系统功能。
 *
 * 本文件包含了程序的入口函数 main，以及摄像头操作、用户管理和实时人脸识别等核心功能。
 * 旨在提供一个基于 OpenCV 和 ONNX 模型的安全摄像头考勤系统。
 */

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <filesystem>

#include "./include/canvas.h"
#include "./include/face_detector.h"
#include "./include/face_recognizer.h"
#include "./include/vector_db.h"

/**
 * @brief 文件系统命名空间别名，方便使用。
 */
namespace fs = std::filesystem;

/**
 * @brief 打开指定设备 ID 的摄像头。
 *
 * 根据不同的操作系统，使用不同的 OpenCV 摄像头后端。
 *
 * @param device_id 摄像头设备 ID，默认为 0。
 * @return cv::VideoCapture 摄像头对象。
 */
cv::VideoCapture open_camera(int device_id = 0) {
#ifdef _WIN32
    return cv::VideoCapture(device_id, cv::CAP_DSHOW);
#elif __APPLE__
    return cv::VideoCapture(device_id, cv::CAP_AVFOUNDATION);
#elif __linux__
    #ifdef __arm__
        return cv::VideoCapture(device_id, cv::CAP_V4L2);
    #else
        return cv::VideoCapture(device_id, cv::CAP_V4L2);
    #endif
#else
    return cv::VideoCapture(device_id);
#endif
}

/**
 * @brief 扫描并查找可用的摄像头。
 *
 * 尝试打开从 0 开始的设备 ID，直到找到一个可用的摄像头或达到最大扫描次数。
 *
 * @param max_scan 最大扫描次数，默认为 5。
 * @return int 可用摄像头的设备 ID，如果未找到则返回 -1。
 */
int find_available_camera(int max_scan = 5) {
    for (int i = 0; i < max_scan; i++) {
        cv::VideoCapture cap = open_camera(i);
        if (cap.isOpened()) {
            cap.release();
            return i;
        }
    }

    return -1;
}

/**
 * @brief 列出所有可用的摄像头。
 *
 * 扫描从 0 到 4 的设备 ID，并打印出所有成功打开的摄像头 ID。
 */
void list_available_cameras() {
    std::cout << "[INFO] 正在扫描可用摄像头..." << std::endl;
    for (int i = 0; i < 5; i++) {
        cv::VideoCapture cap = open_camera(i);
        if (cap.isOpened()) {
            std::cout << "  -> 摄像头 ID: " << i << std::endl;
            cap.release();
        }
    }
}

/**
 * @brief 将用户添加到数据库。
 *
 * 从指定图像中检测人脸，提取人脸特征，并将其与用户 ID 和用户名称一起存储到向量数据库中。
 *
 * @param db 向量数据库对象。
 * @param detector 人脸检测器对象。
 * @param recognizer 人脸识别器对象。
 * @param user_id 用户的唯一 ID。
 * @param user_name 用户的名称。
 * @param img_path 包含用户人脸的图像路径。
 */
void add_user_to_db(
    VectorDB& db, 
    FaceDetector& detector, 
    FaceRecognizer& recognizer, 
    int user_id, 
    const std::string& user_name, 
    const std::string& img_path
) {
    if (!fs::exists(img_path)) {
        std::cerr << "[ERROR] 用户图像不存在: " << img_path << std::endl;
        return;
    }

    cv::Mat img = cv::imread(img_path);
    if (img.empty()) {
        std::cerr << "[ERROR] 无法读取图像: " << img_path << std::endl;
        return;
    }

    auto bboxes = detector.detect(img);
    if (bboxes.empty()) {
        std::cerr << "[ERROR] 未检测到人脸: " << img_path << std::endl;
        return;
    }

    cv::Mat face = img(bboxes[0].toRect()).clone();
    cv::Mat feature = recognizer.extract_feature(face, bboxes[0]);
    db.add_user(user_id, user_name, feature);

    std::cout << "[INFO] 用户 " << user_name << " 已入库" << std::endl;
}

/***
 * @brief 实时人脸识别考勤系统。
 *
 * 打开指定摄像头，实时捕获视频帧，检测人脸，识别人脸并显示考勤信息。
 * 按 'q' 或 'Q' 键退出。
 *
 * @param db 向量数据库对象。
 * @param detector 人脸检测器对象。
 * @param recognizer 人脸识别器对象。
 * @param cam_id 摄像头设备 ID。
 */
void check_in_camera(VectorDB& db, FaceDetector& detector, FaceRecognizer& recognizer, int cam_id) {
    cv::VideoCapture cap = open_camera(cam_id);
    if (!cap.isOpened()) {
        std::cerr << "[FATAL] 无法打开摄像头 ID: " << cam_id << std::endl;
        return;
    }

    std::cout << "[INFO] 摄像头已启动，按 Q 退出" << std::endl;

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        auto bboxes = detector.detect(frame);
        for (auto& box : bboxes) {
            cv::Rect roi = box.toRect();
            cv::rectangle(frame, roi, cv::Scalar(0, 255, 0), 2);

            cv::Mat face = frame(roi).clone();
            cv::Mat feature = recognizer.extract_feature(face, box);
            int user_id = db.search_user(feature);

            std::string label = (user_id != -1) ? "用户ID: " + std::to_string(user_id) : "未知用户";
            cv::putText(frame, label, cv::Point(box.x1(), box.y1() - 5),
                        cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("SecuredCam - 打卡系统", frame);
        char key = static_cast<char>(cv::waitKey(30));
        if (key == 'q' || key == 'Q') break;
    }

    cap.release();
    cv::destroyAllWindows();
}

/**
 * @brief 主函数。
 *
 * 初始化模型、摄像头，添加用户到数据库，并启动实时人脸识别考勤系统。
 *
 * @return int 程序退出码。
 */
int main() {
    try {
        bool models_missing = false;
        
        std::cout << "[INFO] OpenCV 版本: " << CV_VERSION << std::endl;
        
        if (!cv::dnn::DNN_BACKEND_OPENCV) {
            std::cerr << "[FATAL] 当前 OpenCV 未启用 DNN 模块支持" << std::endl;
            return 0;
        }
        
        fs::path yolov5_model("models/yolov5s.onnx");
        fs::path sface_model("models/face_recognition_sface_2021dec.onnx");

        
        if (!fs::exists(yolov5_model)) {
            std::cerr << "[FATAL] YOLOv5 人脸检测模型缺失: " << yolov5_model << std::endl;
            models_missing = true;
        }
        
        if (!fs::exists(sface_model)) {
            std::cerr << "[FATAL] SFace 人脸识别模型缺失: " << sface_model << std::endl;
            models_missing = true;
        }
        
        if (models_missing) {
            std::cerr << "\n[提示] 请运行以下命令下载模型：" << std::endl;
            std::cerr << "  - macOS/Linux: ./scripts/generate_model.sh" << std::endl;
            std::cerr << "  - Windows: .\\scripts\\generate_model.ps1" << std::endl;
        
            return -1;
        }

        list_available_cameras();

        int cam_id = find_available_camera();
        
        if (cam_id == -1) {
            std::cerr << "[FATAL] 找不到可用摄像头!" << std::endl;
            return 0;
        }

        std::cout << "[INFO] 正在加载人脸检测模型..." << std::endl;
        FaceDetector detector(yolov5_model.string());
        std::cout << "[INFO] 人脸检测模型加载完成" << std::endl;
        
        std::cout << "[INFO] 正在加载人脸识别模型..." << std::endl;
        FaceRecognizer recognizer(sface_model.string());
        std::cout << "[INFO] 人脸识别模型加载完成" << std::endl;
        
        VectorDB db;

        add_user_to_db(db, detector, recognizer, 1, "某某人", "mmr.jpg");
        check_in_camera(db, detector, recognizer, cam_id);

        db.save("db");
        std::cout << "[INFO] 数据库已保存" << std::endl;

        return 0;
        
    } catch (const cv::Exception& e) {
        std::cerr << "\n[FATAL] OpenCV 异常: " << e.what() << std::endl;
        std::cerr << "[DEBUG] 错误码: " << e.code << std::endl;
        std::cerr << "[DEBUG] 文件: " << e.file << ":" << e.line << std::endl;
        std::cerr << "[DEBUG] 函数: " << e.func << std::endl;
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "\n[FATAL] 标准异常: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "\n[FATAL] 未知异常" << std::endl;
        return -1;
    }
}