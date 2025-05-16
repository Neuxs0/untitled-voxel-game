CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS =
LDLIBS = -lglfw -lGLEW -lGL -lm -lsoil2
INCLUDE_DIRS =

SRC_DIR = src
OBJ_DIR = build/obj
DIST_DIR = dist
TARGET_NAME = untitled_voxel_game
TARGET = $(DIST_DIR)/$(TARGET_NAME)

SHADER_SRC_DIR = $(SRC_DIR)/shaders
SHADER_DEST_DIR = $(DIST_DIR)/shaders
TEXTURE_SRC_DIR = $(SRC_DIR)/textures
TEXTURE_DEST_DIR = $(DIST_DIR)/textures

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

DIRS_TO_CREATE = $(OBJ_DIR) $(DIST_DIR) $(SHADER_DEST_DIR) $(TEXTURE_DEST_DIR)

.PHONY: all clean run rebuild test copy_shaders copy_textures

all: $(TARGET) copy_shaders copy_textures

$(DIRS_TO_CREATE):
	mkdir -p $@

$(TARGET): $(OBJECTS) | $(DIST_DIR)
	@echo "Linking $@"
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compiling $< to $@"
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

copy_shaders: | $(SHADER_DEST_DIR)
	@echo "Copying shaders to $(SHADER_DEST_DIR)..."
	cp -R $(SHADER_SRC_DIR)/* $(SHADER_DEST_DIR)/

copy_textures: | $(TEXTURE_DEST_DIR)
	@echo "Copying shaders to $(TEXTURE_DEST_DIR)..."
	cp -R $(TEXTURE_SRC_DIR)/* $(TEXTURE_DEST_DIR)/

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR) $(DIST_DIR)

run: all
	@echo "Running $(TARGET_NAME) from $(DIST_DIR)..."
	cd $(DIST_DIR) && ./$(TARGET_NAME)

rebuild: clean all

test: clean all run
