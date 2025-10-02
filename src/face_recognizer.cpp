#include "../include/face_recognizer.h"

FaceRecognizer::FaceRecognizer(const std::string& model_path) {
    recognizer = cv::FaceRecognizerSF::create(model_path, "");
}

cv::Mat FaceRecognizer::extract_feature(const cv::Mat& img, const Bbox& bbox) {
    cv::Rect face_rect(
        std::max(int(bbox.x - bbox.w / 2), 0),
        std::max(int(bbox.y - bbox.h / 2), 0),
        int(bbox.w),
        int(bbox.h)
    );

    face_rect &= cv::Rect(0, 0, img.cols, img.rows);

    cv::Mat face = img(face_rect).clone();
    cv::Mat feature;

    recognizer->feature(face, feature);
    return feature;
}


float FaceRecognizer::compare_features(const cv::Mat& feat1, const cv::Mat& feat2) {
    return recognizer->match(feat1, feat2, cv::FaceRecognizerSF::FR_COSINE);
}
