#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <filesystem>

#include "./include/canvas.h"
#include "./include/face_detector.h"
#include "./include/face_recognizer.h"
#include "./include/vector_db.h"

namespace fs = std::filesystem;

// Cross-platform camera opening
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

// Scan for an available camera
int find_available_camera(int max_scan = 5) {
    for (int i = 0; i < max_scan; i++) {
        cv::VideoCapture cap = open_camera(i);
        if (cap.isOpened()) {
            cap.release();
            return i;
        }
    }

    return 0;
}

// List all available cameras
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

// Add user to the database
void add_user_to_db(VectorDB& db, FaceDetector& detector, FaceRecognizer& recognizer,
                    int user_id, const std::string& user_name, const std::string& img_path) {
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

// Real-time face recognition
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

int main() {
    // Model paths relative to project root
    fs::path yolov5_model("models/yolov5s-face.onnx");
    fs::path sface_model("models/face_recognition_sface_2021dec.onnx");

    // Check models exist
    if (!fs::exists(yolov5_model) || !fs::exists(sface_model)) {
        std::cerr << "[FATAL] 模型缺失! 请先运行 scripts/generate_model.sh 或 scripts/generate_model.ps1" << std::endl;
        return -1;
    }

    list_available_cameras();

    int cam_id = find_available_camera();
    if (cam_id == -1) {
        std::cerr << "[FATAL] 找不到可用摄像头!" << std::endl;
        return -1;
    }

    FaceDetector detector(yolov5_model.string());
    FaceRecognizer recognizer(sface_model.string());
    VectorDB db;


    add_user_to_db(db, detector, recognizer, 1, "某某人", "mmr.jpg");

    check_in_camera(db, detector, recognizer, cam_id);

    db.save("db");
    std::cout << "[INFO] 数据库已保存" << std::endl;

    return 0;
}
