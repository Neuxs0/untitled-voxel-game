CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O3 -MMD -MP -DGLM_ENABLE_EXPERIMENTAL
LDFLAGS =
LDLIBS = -lglfw -lGLEW -lGL -lm

SRC_DIR = src
OBJ_DIR = build/obj
DIST_DIR = dist
TARGET_NAME = untitled_voxel_game
TARGET = $(DIST_DIR)/$(TARGET_NAME)

ASSETS_SRC_DIR = $(SRC_DIR)/assets

# Add the source directory to the include path
INCLUDE_DIRS = -I$(SRC_DIR)

SOURCES = $(shell find $(SRC_DIR) -name '*.cpp')

# Find all .glsl files, using -o to prevent duplicates for names like *.comp.glsl.
SHADER_FILES = $(shell find $(ASSETS_SRC_DIR)/shaders -name '*.glsl' -o -name '*.comp.glsl')

# Generate object file names for shaders.
SHADER_OBJECTS = $(patsubst $(ASSETS_SRC_DIR)/shaders/%,$(OBJ_DIR)/shaders/%.o,$(SHADER_FILES))

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

.PHONY: all clean run rebuild test test_sequence

# Default target: build the executable.
all: $(TARGET)

# Create the necessary directories if they don't exist.
$(OBJ_DIR) $(DIST_DIR) $(OBJ_DIR)/shaders:
	$(Q)mkdir -p $@

# Link all object files (C++ and shader-generated) to create the executable.
$(TARGET): $(OBJECTS) $(SHADER_OBJECTS) | $(DIST_DIR)
	$(VECHO) "Linking $@"
	$(Q)$(CXX) $(OBJECTS) $(SHADER_OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

# Compile source files into object files.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(VECHO) "Compiling $< to $@"
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Rule to convert a file into a linkable object file using the linker (ld).
$(OBJ_DIR)/shaders/%.o: $(ASSETS_SRC_DIR)/shaders/% | $(OBJ_DIR)/shaders
	$(VECHO) "Embedding $<"
	$(Q)ld -r -b binary -o $@ $<

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
