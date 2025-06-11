CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -MMD -MP
LDFLAGS =
LDLIBS = -lglfw -lGLEW -lGL -lm
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
DEPS = $(OBJECTS:.o=.d)

# Make the output silent by default
ifeq ($(SILENT),true)
    Q := @
    VECHO := @:
else
    Q :=
    VECHO := @echo
endif

.PHONY: all clean run rebuild test copy_assets test_sequence

# Default target: build the executable and copy assets.
all: $(TARGET) copy_assets

# Create the object directory if it doesn't exist.
$(OBJ_DIR):
	$(Q)mkdir -p $@

# Create the distribution directory if it doesn't exist.
$(DIST_DIR):
	$(Q)mkdir -p $@

# Create the assets destination directory if it doesn't exist.
$(ASSETS_DEST_DIR):
	$(Q)mkdir -p $@

# Link the object files to create the executable.
$(TARGET): $(OBJECTS) | $(DIST_DIR)
	$(VECHO) "Linking $@"
	$(Q)$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

# Compile source files into object files.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(VECHO) "Compiling $< to $@"
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Copy assets to the distribution directory.
copy_assets: | $(ASSETS_DEST_DIR)
	$(VECHO) "Copying assets to $(ASSETS_DEST_DIR)..."
	$(Q)cp -R $(ASSETS_SRC_DIR)/* $(ASSETS_DEST_DIR)/

# Clean up all build artifacts.
clean:
	$(VECHO) "Cleaning build artifacts..."
	$(Q)rm -rf $(OBJ_DIR) $(DIST_DIR)

# Run the game.
run: all
	$(VECHO) "Running $(TARGET_NAME) from $(DIST_DIR)..."
	$(Q)cd $(DIST_DIR) && ./$(TARGET_NAME)

# Perform a clean build.
rebuild: clean all

# A sequence of cleaning, building, and running.
test_sequence: clean all run

# Execute the test_sequence silently.
test:
	@$(MAKE) --no-print-directory SILENT=true test_sequence

# Include all dependency files.
-include $(DEPS)
