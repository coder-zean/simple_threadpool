BUILD_DIR = ./build
DEST_DIR = ./out
DEST_EXE_NAME = hello

all: build
	cd $(BUILD_DIR); make -j4
  
build:
	mkdir $(BUILD_DIR); cd $(BUILD_DIR); cmake ..
  
run:
	cd $(DEST_DIR); ./$(DEST_EXE_DIR)
  
clean:
	rm $(BUILD_DIR) $(DEST_DIR) -rf