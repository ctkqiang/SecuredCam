#!/usr/bin/env bash

MODELS_DIR="./models"
YOLO_REPO_DIR="./yolov5"
PT_FILE="yolov5s-face.pt"
ONNX_FILE="yolov5s-face.onnx"
SFACE_FILE="face_recognition_sface_2021dec.onnx"

echo "=================================================="
echo " SecuredCam 模型下载与准备工具"
echo "=================================================="

mkdir -p "$MODELS_DIR" || { echo "[错误] 无法创建 $MODELS_DIR"; exit 1; }

# 下载 SFace 模型
if [ ! -f "$MODELS_DIR/$SFACE_FILE" ]; then
    wget -c "https://mirror.its.dal.ca/kde-applicationdata/digikam/facesengine/dnnface/$SFACE_FILE" -O "$MODELS_DIR/$SFACE_FILE" \
        || { echo "[错误] 下载 SFace 模型失败"; exit 1; }
else
    echo "[INFO] 已存在: $MODELS_DIR/$SFACE_FILE"
fi

# 下载 YOLOv5s .pt 权重
if [ ! -f "$MODELS_DIR/$PT_FILE" ]; then
    wget -c "https://huggingface.co/Ultralytics/YOLOv5/resolve/main/yolov5s.pt" -O "$MODELS_DIR/$PT_FILE" \
        || { echo "[错误] 下载 YOLOv5s 权重失败"; exit 1; }
else
    echo "[INFO] 已存在: $MODELS_DIR/$PT_FILE"
fi

# 导出 ONNX 模型
if [ ! -f "$MODELS_DIR/$ONNX_FILE" ]; then
    if [ ! -d "$YOLO_REPO_DIR" ]; then
        git clone https://github.com/ultralytics/yolov5.git --depth=1 "$YOLO_REPO_DIR" \
            || { echo "[错误] 克隆 YOLOv5 仓库失败"; exit 1; }
    fi

    cd "$YOLO_REPO_DIR" || { echo "[错误] 进入 $YOLO_REPO_DIR 失败"; exit 1; }

    python3 -m venv venv || { echo "[错误] 创建 venv 失败"; exit 1; }
    source venv/bin/activate
    pip install --upgrade pip >/dev/null
    pip install -r requirements.txt >/dev/null

    echo "[INFO] 正在导出 ONNX 模型..."
    python3 export.py --weights "../$MODELS_DIR/$PT_FILE" --include onnx --dynamic \
        || { 
            echo "[WARN] export.py 导出失败，尝试直接下载 ONNX 模型"; 
            cd ..
            wget -c "https://huggingface.co/amd/yolov5s/resolve/main/yolov5s.onnx" -O "$MODELS_DIR/$ONNX_FILE" \
                || { echo "[错误] 下载 ONNX 模型失败"; exit 1; }
            echo "[INFO] 已通过下载获取 ONNX 模型"
            exit 0
        }

    ONNX_TMP=$(find . -maxdepth 2 -name "*.onnx" | head -n 1)
    if [ -f "$ONNX_TMP" ]; then
        mv "$ONNX_TMP" "../$MODELS_DIR/$ONNX_FILE"
        echo "[INFO] 已生成 ONNX 模型: $MODELS_DIR/$ONNX_FILE"
    else
        echo "[WARN] 未生成 ONNX 模型，尝试下载"
        cd ..
        wget -c "https://huggingface.co/amd/yolov5s/resolve/main/yolov5s.onnx" -O "$MODELS_DIR/$ONNX_FILE" \
            || { echo "[错误] 下载 ONNX 模型失败"; exit 1; }
        echo "[INFO] 已通过下载获取 ONNX 模型"
    fi

    deactivate
    cd ..
else
    echo "[INFO] 已存在: $MODELS_DIR/$ONNX_FILE"
fi

echo "=================================================="
echo " 模型已准备完成，保存在 $MODELS_DIR/"
echo "=================================================="
