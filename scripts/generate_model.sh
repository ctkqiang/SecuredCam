#!/usr/bin/env bash

MODELS_DIR="./models"
YOLO_REPO_DIR="./yolov5"
PT_FILE="yolov5s.pt"
ONNX_FILE="yolov5s.onnx"
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
download_pt_model() {
    local urls=(
        "https://github.com/ultralytics/yolov5/releases/download/v7.0/yolov5s.pt"
    )
    
    for url in "${urls[@]}"; do
        echo "[INFO] 尝试从 $url 下载模型..."
        if wget -c "$url" -O "$MODELS_DIR/$PT_FILE" 2>/dev/null; then
            echo "[INFO] 成功下载 YOLOv5s 权重"
            return 0
        fi
    done
    
    echo "[错误] 所有下载源均失败"
    return 1
}

if [ ! -f "$MODELS_DIR/$PT_FILE" ]; then
    download_pt_model || { 
        echo "[提示] 请手动下载 YOLOv5s 模型:";
        echo "1. 访问: https://github.com/ultralytics/yolov5/releases/tag/v7.0";
        echo "2. 下载 yolov5s.pt";
        echo "3. 将文件放置在 $MODELS_DIR/ 目录下";
        exit 1;
    }
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
    python3 export.py --weights "../$MODELS_DIR/$PT_FILE" --include onnx --dynamic --opset 12 \
        || { 
            echo "[ERROR] export.py 导出失败，请检查 Python/torch/onnx 依赖并重试"; 
            echo "[提示] 你也可以手动导出: cd yolov5 && python3 export.py --weights ../models/yolov5s.pt --include onnx --dynamic --opset 12";
            deactivate 2>/dev/null || true;
            cd ..;
            exit 1;
        }

    if [ -f "../$MODELS_DIR/$ONNX_FILE" ]; then
        echo "[INFO] ONNX 模型已成功导出: $MODELS_DIR/$ONNX_FILE"
    else
        echo "[ERROR] ONNX 模型导出失败"
        cd ..
        echo "[ERROR] 未发现导出的 ONNX 文件，请重试导出步骤";
        deactivate 2>/dev/null || true;
        cd ..;
        exit 1;
    fi

    deactivate
    cd ..
else
    echo "[INFO] 已存在: $MODELS_DIR/$ONNX_FILE"
fi

# 验证模型文件完整性
verify_models() {
    local expected_sface_size=38696353  # SFace 模型预期大小（字节）
    local expected_pt_min=14000000      # YOLOv5s.pt 预期大小下限（字节）
    local expected_pt_max=16000000      # YOLOv5s.pt 预期大小上限（字节）
    
    echo "[INFO] 验证模型文件完整性..."
    
    # 验证 SFace 模型
    if [ -f "$MODELS_DIR/$SFACE_FILE" ]; then
        local sface_size=$(wc -c < "$MODELS_DIR/$SFACE_FILE")
        if [ "$sface_size" -eq "$expected_sface_size" ]; then
            echo "[INFO] SFace 模型验证通过"
        else
            echo "[WARN] SFace 模型大小异常，可能已损坏"
            return 1
        fi
    else
        echo "[ERROR] SFace 模型文件缺失"
        return 1
    fi
    
    # 验证 YOLOv5s 模型
    if [ -f "$MODELS_DIR/$PT_FILE" ]; then
        local pt_size=$(wc -c < "$MODELS_DIR/$PT_FILE")
        if [ "$pt_size" -ge "$expected_pt_min" ] && [ "$pt_size" -le "$expected_pt_max" ]; then
            echo "[INFO] YOLOv5s 模型验证通过"
        else
            echo "[WARN] YOLOv5s 模型大小异常，可能需要重新下载"
            return 1
        fi
    else
        echo "[ERROR] YOLOv5s 模型文件缺失"
        return 1
    fi
    
    return 0
}

verify_models
if [ $? -eq 0 ]; then
    echo "=================================================="
    echo " 模型已准备完成，保存在 $MODELS_DIR/"
    echo "=================================================="
else
    echo "=================================================="
    echo " [警告] 模型文件可能存在问题，请考虑重新下载或重新导出 ONNX"
    echo "=================================================="
fi