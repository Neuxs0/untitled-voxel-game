CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra -O2

LDFLAGS =

LDLIBS = -lglfw -lGLEW -lGL -lm

INCLUDE_DIRS =

SRC_DIR = src
OBJ_DIR = build/obj
TARGET_NAME = untitled_voxel_game
TARGET = dist/$(TARGET_NAME)

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	@echo "Linking $@"
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	@echo "Compiling $< to $@"
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: run
run: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

.PHONY: rebuild
rebuild: clean all

.PHONY: test
test: clean all run
