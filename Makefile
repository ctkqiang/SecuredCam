PROJECT_NAME = SecuredCam
BUILD_DIR = build

ifeq ($(OS),Windows_NT)
    MODEL_SCRIPT = powershell.exe -ExecutionPolicy Bypass -File scripts/generate_model.ps1
    RM = rmdir /S /Q
    MKDIR = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
else
    MODEL_SCRIPT = sh scripts/generate_model.sh
    RM = rm -rf
    MKDIR = mkdir -p $(BUILD_DIR)
endif

.PHONY: all run models clean

all: $(BUILD_DIR)/$(PROJECT_NAME)

$(BUILD_DIR)/$(PROJECT_NAME):
	$(MKDIR)
	cd $(BUILD_DIR) && cmake .. && make -j$$(nproc)

run: check_models all
	./$(BUILD_DIR)/$(PROJECT_NAME)

check_models:
	@if [ ! -d models ] || [ ! -f models/yolov5s-face.onnx ] || [ ! -f models/face_recognition_sface_2021dec.onnx ]; then \
		echo "[INFO] 模型缺失，正在生成..."; \
		$(MODEL_SCRIPT); \
	else \
		echo "[INFO] 模型已存在"; \
	fi

models:
	$(MODEL_SCRIPT)

clean:
	$(RM) $(BUILD_DIR)
	$(RM) models
	$(RM) yolov5
