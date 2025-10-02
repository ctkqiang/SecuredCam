PROJECT_NAME = SecuredCam
BUILD_DIR = build

ifeq ($(OS),Windows_NT)
    MODEL_SCRIPT = powershell.exe -ExecutionPolicy Bypass -File scripts/generate_model.ps1
else
    MODEL_SCRIPT = sh scripts/generate_model.sh
endif

all: $(BUILD_DIR)/$(PROJECT_NAME)

$(BUILD_DIR)/$(PROJECT_NAME):
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make -j$$(sysctl -n hw.ncpu)

run: all
	./$(BUILD_DIR)/$(PROJECT_NAME)

models:
	$(MODEL_SCRIPT)

clean:
	rm -rf $(BUILD_DIR) && rm -rf models && rm -rf yolov5
