#pragma once  

#include <faiss/index_io.h>  
#include <faiss/IndexFlat.h>  

#include <vector>  
#include <string> 

#include "user.h"

class VectorDB {  
private:  
    faiss::IndexFlatL2 index;  
    std::vector<User> users;   

public:  
    VectorDB(int dim = 128);

    int search_user(const cv::Mat& feature, float threshold = 0.6f);

    void add_user(int id, const std::string& name, const cv::Mat& feature); 
    void save(const std::string& path);
    void load(const std::string& path);  
};