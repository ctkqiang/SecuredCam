<#
.SYNOPSIS
    人脸识别模型下载与准备脚本 (PowerShell 版)

.DESCRIPTION
    本脚本会自动完成以下工作：
      1. 创建 models 目录
      2. 下载 SFace 人脸识别 ONNX 模型
      3. 下载 YOLOv5s-face 人脸检测 PyTorch 权重
      4. 克隆 yolov5-face 仓库
      5. 创建 Python 虚拟环境并安装依赖
      6. 将 YOLOv5s-face 导出为 ONNX 模型

      本脚本仅用于研究与学习目的，请遵循所使用模型和代码仓库的开源协议。
        - SFace 模型来自 KDE/digiKam 项目
        - YOLOv5-face 来自 deepcam-cn/yolov5-face 项目

.AUTHOR
    钟智强
#>

$ModelsDir = "models"
$RepoDir = "yolov5-face"
$PtFile = "yolov5s-face.pt"
$OnnxFile = "yolov5s-face.onnx"
$SfaceFile = "face_recognition_sface_2021dec.onnx"

function Print-Line {
    Write-Host ("=" * 50) -ForegroundColor Cyan
}

function Print-Step($msg) {
    Write-Host "[步骤] $msg" -ForegroundColor Green
}

function Print-Error($msg) {
    Write-Host "[错误] $msg" -ForegroundColor Red
}

# 主流程
Print-Line
Write-Host " 人脸模型下载与准备工具 (PowerShell)" -ForegroundColor Yellow
Print-Line

# Step 1: 创建 models 目录
if (-not (Test-Path $ModelsDir)) {
    New-Item -ItemType Directory -Path $ModelsDir | Out-Null
}

# Step 2: 下载 SFace 模型
Print-Step "下载 SFace ONNX 模型..."
Invoke-WebRequest -Uri "https://mirror.its.dal.ca/kde-applicationdata/digikam/facesengine/dnnface/$SfaceFile" `
                  -OutFile "$ModelsDir\$SfaceFile" `
                  -UseBasicParsing

# Step 3: 下载 YOLOv5s-face 权重
Print-Step "下载 YOLOv5s-face 权重 (.pt)..."
Invoke-WebRequest -Uri "https://github.com/deepcam-cn/yolov5-face/releases/download/v1.0/$PtFile" `
                  -OutFile "$ModelsDir\$PtFile" `
                  -UseBasicParsing

# Step 4: 克隆 yolov5-face 仓库
if (-not (Test-Path $RepoDir)) {
    Print-Step "克隆 yolov5-face 仓库..."
    git clone https://github.com/deepcam-cn/yolov5-face.git --depth=1
}

Set-Location $RepoDir

# Step 5: 设置 Python 虚拟环境
Print-Step "创建并激活 Python 虚拟环境..."
python -m venv venv
& .\venv\Scripts\Activate.ps1
pip install --upgrade pip | Out-Null
pip install -r requirements.txt

# Step 6: 导出 YOLOv5 模型为 ONNX
Print-Step "将 YOLOv5s-face 导出为 ONNX..."
python export.py --weights "..\$ModelsDir\$PtFile" --include onnx --dynamic --opset 12

# 移动 ONNX 文件到 models 目录
Move-Item -Path $OnnxFile -Destination "..\$ModelsDir\" -Force

# 返回项目根目录
Set-Location ..

# 最终提示
Print-Line
Print-Step " 所有模型已准备完成，保存在 $ModelsDir\"
Print-Line
