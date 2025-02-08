# Variables
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
CXXFLAGS += $(shell pkg-config --cflags --libs gtkmm-3.0)
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/mierucache

# Find all source files
SRCS = $(wildcard $(SRC_DIR)/*.cc)
OBJS = $(patsubst $(SRC_DIR)/%.cc, $(BUILD_DIR)/%.o, $(SRCS))

# Default target
all: $(TARGET)

# Link the target executable
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
