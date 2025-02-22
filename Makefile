CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./include \
           -I/opt/homebrew/include \
           -I/usr/local/include

LDFLAGS = -L/opt/homebrew/lib \
          -L/usr/local/lib \
          -lgtest \
          -lgtest_main \
          -pthread

SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build
BIN_DIR = bin
TEST_BUILD_DIR = $(BUILD_DIR)/tests

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)

# Filter out main.cpp from object files when building tests
MAIN_SRC = $(SRC_DIR)/main.cpp
BASE_SRCS = $(filter-out $(MAIN_SRC),$(SRCS))
BASE_OBJS = $(BASE_SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
MAIN_OBJ = $(MAIN_SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Test objects
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(TEST_BUILD_DIR)/%.o)

# Targets
TARGET = $(BIN_DIR)/6502emu
TEST_TARGET = $(BIN_DIR)/run_tests

# Create directories
$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(BIN_DIR))
$(shell mkdir -p $(TEST_BUILD_DIR))

# Default target
all: $(TARGET) $(TEST_TARGET)

# Main program
$(TARGET): $(BASE_OBJS) $(MAIN_OBJ)
	$(CXX) $^ -o $@

# Test program
$(TEST_TARGET): $(BASE_OBJS) $(TEST_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test files
$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: clean test all
