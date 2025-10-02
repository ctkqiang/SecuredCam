#include "../include/vector_db.h"  

VectorDB::VectorDB(int dim) : index(dim) {
    // ???
}   

void VectorDB::add_user(int id, const std::string& name, const cv::Mat& feature) {  
    // 将OpenCV Mat转换为FAISS格式（float数组
    std::vector<float> feat_vec;  
    feat_vec.assign((float*)feature.data, (float*)feature.data + feature.total());  

    // 插入向量库  
    index.add(1, feat_vec.data());  
    users.push_back({id, name, feature});  
}  

void VectorDB::save(const std::string& path) {  
    faiss::write_index(&index, (path + "/index.bin").c_str());  
}  

void VectorDB::load(const std::string& path) {  
    faiss::Index* loaded_index = faiss::read_index((path + "/index.bin").c_str());  
    index = *(dynamic_cast<faiss::IndexFlatL2*>(loaded_index));  
}

int VectorDB::search_user(const cv::Mat& feature, float threshold) {
    if (users.empty()) return -1;

    int k = 1;
    int result = -1;

    cv::Mat feat = feature.isContinuous() ? feature : feature.clone();
    std::vector<float> feat_vec(feat.begin<float>(), feat.end<float>());

    std::vector<faiss::idx_t> I(k);
    std::vector<float> D(k);

    index.search(1, feat_vec.data(), k, D.data(), I.data());

    if (!I.empty() && I[0] >= 0) {
        if (D[0] < threshold) result = users[I[0]].id;
    }
    
    return result;
}

