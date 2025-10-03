#include "../include/face_detector.h"  

/**
 * @brief 构造函数，加载YOLO ONNX模型
 * 
 * @param model_path ONNX模型路径
 */
FaceDetector::FaceDetector(const std::string& model_path) {  
    net = cv::dnn::readNetFromONNX(model_path);  
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);  
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU); 
}  

/**
 * @brief 构造函数，使用预配置的 Net 对象
 * 
 * @param net 预配置的 OpenCV DNN Net 对象
 */
FaceDetector::FaceDetector(const cv::dnn::Net& net) : net(net) {}

std::vector<Bbox> FaceDetector::detect(const cv::Mat& img) {
    std::vector<Bbox> bboxes;
    std::vector<cv::Rect> cv_boxes;
    std::vector<float> scores;

    int img_w = img.cols;
    int img_h = img.rows;

    cv::Mat blob;
    cv::dnn::blobFromImage(img, blob, 1/255.0, cv::Size(640, 640), cv::Scalar(0,0,0), true, false);
    net.setInput(blob);

    cv::Mat output;
    net.forward(output);

    cv::Mat det = output.reshape(1, output.total() / 85);

    for (int i = 0; i < det.rows; i++) {
        float conf = det.at<float>(i, 4);
        if (conf < conf_threshold) continue;

        float x = det.at<float>(i, 0);
        float y = det.at<float>(i, 1);
        float w = det.at<float>(i, 2);
        float h = det.at<float>(i, 3);

        // 将相对坐标转换为绝对坐标
        x = x * img_w;
        y = y * img_h;
        w = w * img_w;
        h = h * img_h;

        Bbox box = {
            .x = x,
            .y = y,
            .w = w,
            .h = h,
            .conf = conf
        };
        bboxes.push_back(box);

        cv_boxes.push_back(cv::Rect((int)(x - w/2), (int)(y - h/2), (int)w, (int)h));
        scores.push_back(conf);
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(cv_boxes, scores, conf_threshold, nms_threshold, indices);

    std::vector<Bbox> result;
    for (int idx : indices) {
        result.push_back(bboxes[idx]);
    }

    return result;
}
