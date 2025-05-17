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

ifeq ($(SILENT),true)
    Q := @
    VECHO := @:
else
    Q :=
    VECHO := @echo
endif

.PHONY: all clean run rebuild test copy_assets test_sequence

# Default target
all: $(TARGET) copy_assets

$(OBJ_DIR):
	$(Q)mkdir -p $@

$(DIST_DIR):
	$(Q)mkdir -p $@

$(ASSETS_DEST_DIR):
	$(Q)mkdir -p $@

# Link the target executable
$(TARGET): $(OBJECTS) | $(DIST_DIR)
	$(VECHO) "Linking $@"
	$(Q)$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(VECHO) "Compiling $< to $@"
	$(Q)mkdir -p $(dir $@) # Ensure specific subdirectory for .o file exists (e.g., build/obj/utils/)
	$(Q)$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Copy assets
copy_assets: | $(ASSETS_DEST_DIR)
	$(VECHO) "Copying assets to $(ASSETS_DEST_DIR)..."
	$(Q)cp -R $(ASSETS_SRC_DIR)/* $(ASSETS_DEST_DIR)/

# Clean build artifacts
clean:
	$(VECHO) "Cleaning build artifacts..."
	$(Q)rm -rf $(OBJ_DIR) $(DIST_DIR)

# Run the game
run: all
	$(VECHO) "Running $(TARGET_NAME) from $(DIST_DIR)..."
	$(Q)cd $(DIST_DIR) && ./$(TARGET_NAME)

# Clean then build all
rebuild: clean all

test_sequence: clean all run

# Executes the test_sequence silently
test:
	@$(MAKE) --no-print-directory SILENT=true test_sequence
