#include "../include/face_detector.h"  

/**
 * @brief 构造函数，加载YOLO ONNX模型
 * 
 * @param model_path ONNX模型路径
 */
FaceDetector::FaceDetector(const std::string& model_path) {  
    net = cv::dnn::readNetFromONNX(model_path);  
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);  

    // 若有GPU可换DNN_TARGET_CUDA? 
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU); 
}  

std::vector<Bbox> FaceDetector::detect(const cv::Mat& img) {
    std::vector<Bbox> bboxes;
    std::vector<cv::Rect> cv_boxes;
    std::vector<float> scores;

    int img_w = img.cols;
    int img_h = img.rows;

    cv::Mat blob;
    cv::dnn::blobFromImage(img, blob, 1/255.0, cv::Size(640, 640), cv::Scalar(0,0,0), true, false);
    net.setInput(blob);

    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    float* data = (float*)outputs[0].data;
    int rows = outputs[0].rows;

    for (int i = 0; i < rows; ++i) {
        
        float conf = data[i * 6 + 4];
        if (conf < conf_threshold) continue;

        float x = data[i * 6 + 0] * img_w;
        float y = data[i * 6 + 1] * img_h;
        float w = data[i * 6 + 2] * img_w;
        float h = data[i * 6 + 3] * img_h;

        Bbox box = {x, y, w, h, conf};
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
