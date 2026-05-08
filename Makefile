CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Isrc
SRC_DIR  := src
BIN_DIR  := bin
TARGET   := $(BIN_DIR)/solver
SOURCES  := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS  := $(SOURCES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BIN_DIR)
