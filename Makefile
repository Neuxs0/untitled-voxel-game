CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS =
LDLIBS = -lglfw -lGLEW -lGL -lm -lsoil2
INCLUDE_DIRS = -I$(SRC_DIR)

SRC_DIR = src
OBJ_DIR = build/obj
DIST_DIR = dist
TARGET_NAME = untitled_voxel_game
TARGET = $(DIST_DIR)/$(TARGET_NAME)

ASSETS_SRC_DIR = $(SRC_DIR)/assets
ASSETS_DEST_DIR = $(DIST_DIR)/assets

SOURCES = $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

DIRS_TO_CREATE = $(OBJ_DIR) $(DIST_DIR) $(ASSETS_DEST_DIR)

.PHONY: all clean run rebuild test copy_assets

all: $(TARGET) copy_assets

$(DIRS_TO_CREATE):
	mkdir -p $@

$(TARGET): $(OBJECTS) | $(DIST_DIR)
	@echo "Linking $@"
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compiling $< to $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

copy_assets: | $(ASSETS_DEST_DIR)
	@echo "Copying assets to $(ASSETS_DEST_DIR)..."
	mkdir -p $(ASSETS_DEST_DIR)
	cp -R $(ASSETS_SRC_DIR)/* $(ASSETS_DEST_DIR)/

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR) $(DIST_DIR)

run: all
	@echo "Running $(TARGET_NAME) from $(DIST_DIR)..."
	cd $(DIST_DIR) && ./$(TARGET_NAME)

rebuild: clean all

test: clean all run
