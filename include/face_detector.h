#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

#include "canvas.h"

class FaceDetector {
    private:
        cv::dnn::Net net;  

        float conf_threshold = 0.5f;  // 置信度阈值  
        float nms_threshold = 0.4f;   // NMS阈值  

    public:
        FaceDetector(const std::string& model_path);
        FaceDetector(const cv::dnn::Net& net);  // 新增构造函数
        std::vector<Bbox> detect(const cv::Mat& frame_img);
};