#pragma once  

#include <opencv2/opencv.hpp>  
#include <vector>  

#include "face_detector.h"

class FaceRecognizer {  
private:  
    cv::Ptr<cv::FaceRecognizerSF> recognizer;

public:  
    FaceRecognizer(const std::string& model_path);
     
    cv::Mat extract_feature(const cv::Mat& img, const Bbox& bbox);   
    float compare_features(const cv::Mat& feat1, const cv::Mat& feat2);  
};  