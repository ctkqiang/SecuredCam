#include <iostream>
#include <opencv2/opencv.hpp>

int main() {
    // 打印 OpenCV 版本信息
    std::cout << "[信息] OpenCV 版本: " << CV_VERSION << std::endl;

    // 尝试打开摄像头
    cv::VideoCapture cap;
    
    std::cout << "[信息] 尝试打开摄像头..." << std::endl;
    
    // 首先尝试默认摄像头
    cap.open(0, cv::CAP_AVFOUNDATION);
    
    if (!cap.isOpened()) {
        std::cout << "[警告] 默认摄像头打开失败，尝试其他摄像头..." << std::endl;
        // 尝试其他摄像头
        for (int i = 1; i < 2; i++) {
            cap.open(i, cv::CAP_AVFOUNDATION);
            if (cap.isOpened()) {
                std::cout << "[信息] 成功打开摄像头 ID: " << i << std::endl;
                break;
            }
        }
    }

    if (!cap.isOpened()) {
        std::cerr << "[错误] 无法打开任何摄像头" << std::endl;
        return -1;
    }

    // 获取并打印摄像头的一些属性
    double fps = cap.get(cv::CAP_PROP_FPS);
    int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    
    std::cout << "[信息] 摄像头已打开" << std::endl;
    std::cout << "[信息] 分辨率: " << width << "x" << height << std::endl;
    std::cout << "[信息] FPS: " << fps << std::endl;
    std::cout << "[信息] 按 'Q' 键退出" << std::endl;

    cv::Mat frame;
    int frame_count = 0;
    
    while (true) {
        bool success = cap.read(frame);
        
        if (!success || frame.empty()) {
            std::cerr << "[错误] 无法获取视频帧 (帧计数: " << frame_count << ")" << std::endl;
            // 尝试重新初始化摄像头
            cap.release();
            cv::waitKey(1000); // 等待1秒
            if (!cap.open(0, cv::CAP_AVFOUNDATION)) {
                std::cerr << "[错误] 重新初始化摄像头失败" << std::endl;
                break;
            }
            continue;
        }

        frame_count++;
        
        // 显示帧计数
        cv::putText(frame, 
                    "Frame: " + std::to_string(frame_count), 
                    cv::Point(10, 30), 
                    cv::FONT_HERSHEY_SIMPLEX, 
                    1.0, 
                    cv::Scalar(0, 255, 0), 
                    2);
        
        // 显示视频帧
        cv::imshow("摄像头测试", frame);
        
        // 检查是否按下 'q' 键
        char key = (char)cv::waitKey(1);
        if (key == 'q' || key == 'Q')
            break;
    }
    
    std::cout << "[信息] 总共捕获了 " << frame_count << " 帧" << std::endl;
    
    cap.release();
    cv::destroyAllWindows();
    return 0;
}