# SecuredCam

## 项目简介

SecuredCam 是一个面向嵌入式设备的人脸检测与识别系统，基于 OpenCV DNN 模块、YOLOv5-Face、SFace 模型以及 FAISS 向量数据库实现。项目旨在提供轻量化、可移植的人脸识别解决方案，适用于 Raspberry Pi、NVIDIA Jetson、NVR 等嵌入式场景。

系统工作流程：

```
输入图像 / 视频流 → YOLOv5-Face 检测人脸 → SFace 提取特征向量 → FAISS 相似度检索 → 输出识别结果
```

典型应用场景包括门禁系统、考勤设备和嵌入式安防摄像头。

## 系统结构

```
SecuredCam
├── include/                
│   ├── face_detector.h
│   ├── face_recognizer.h
│   ├── vector_db.h
│   └── user.h
├── src/
│   ├── face_detector.cpp
│   ├── face_recognizer.cpp
│   └── vector_db.cpp
├── models/                  # 模型存放目录
├── main.cpp
├── CMakeLists.txt
├── Makefile
└── README.md
```

## 依赖

* C++17 编译器
* CMake ≥ 3.12
* OpenCV ≥ 4.5（需包含 dnn 模块和 opencv_contrib）
* FAISS
* Python 3.8+（仅用于模型转换）
* Git, wget 或 curl

### 安装依赖

#### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y build-essential cmake libopencv-dev libfaiss-dev python3-venv git wget
```

#### macOS (Homebrew)

```bash
brew install cmake opencv faiss
```

#### Windows (vcpkg)

```powershell
vcpkg install opencv faiss
```

## 模型准备

SecuredCam 使用两个模型：

* `yolov5s-face.onnx` — 人脸检测模型
* `face_recognition_sface_2021dec.onnx` — 人脸识别模型

推荐使用提供的脚本自动下载和准备模型：

* **Linux/macOS:** `sh scripts/generate_model.sh`
* **Windows:** `powershell.exe -ExecutionPolicy Bypass -File scripts/generate_model.ps1`

> **注意:** 模型将存放在 `./models` 目录中。如果目录为空，请确保运行了上述脚本。

## 编译与运行

### 使用 CMake

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./SecuredCam
```

### 使用 Makefile

```bash
make
./SecuredCam
```

或直接运行：

```bash
make run
```

> **Tip:** 确保先执行 `make models` 或相应的脚本下载模型。

## 算法说明

### 人脸检测 (YOLOv5-Face)

YOLOv5-Face 基于 YOLOv5 优化，用于高效的人脸检测。输出每个检测框的 `[x, y, w, h, conf, cls]` 信息，其中 `(x, y)` 为中心坐标，`(w, h)` 为宽高，`conf` 为置信度。

### 人脸特征提取 (SFace)

SFace 模型对人脸区域生成 128 维嵌入向量，便于在欧氏空间或余弦空间进行相似度比较。

### 向量检索 (FAISS)

FAISS 提供快速的向量检索能力。已注册用户的特征向量存入索引，新输入的人脸通过最近邻搜索匹配到最相似用户，根据阈值判断识别结果。

