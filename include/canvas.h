#pragma once

#include <opencv2/opencv.hpp>

struct Bbox {
    float x;      // 中心点x坐标
    float y;      // 中心点y坐标
    float w;      // 宽度
    float h;      // 高度
    float conf;   // 置信度

    // 转换为OpenCV的Rect格式
    cv::Rect toRect() const {
        return cv::Rect(
            static_cast<int>(x - w/2),  // 左上角x
            static_cast<int>(y - h/2),  // 左上角y
            static_cast<int>(w),        // 宽度
            static_cast<int>(h)         // 高度
        );
    }

    // 获取左上角和右下角坐标
    int x1() const { return static_cast<int>(x - w/2); }
    int y1() const { return static_cast<int>(y - h/2); }
    int x2() const { return static_cast<int>(x + w/2); }
    int y2() const { return static_cast<int>(y + h/2); }
};
