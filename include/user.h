#include <string>
#include <opencv2/opencv.hpp>

struct User {  
    int id;               // 用户ID  
    std::string name;     // 用户名  
    cv::Mat feature;      // 人脸特征向量  
};  