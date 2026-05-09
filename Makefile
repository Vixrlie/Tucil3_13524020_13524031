CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Isrc
SRC_DIR  := src
BIN_DIR  := bin
TARGET   := $(BIN_DIR)/solver
SOURCES  := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS  := $(SOURCES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    BREW_PREFIX := $(shell brew --prefix 2>/dev/null)
    ifneq ($(BREW_PREFIX),)
        CXXFLAGS += -I$(BREW_PREFIX)/include
        LDFLAGS  += -L$(BREW_PREFIX)/lib
    endif
    LDLIBS   += -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreAudio -framework CoreFoundation
else
    PKG_RAYLIB := $(shell pkg-config --cflags --libs raylib 2>/dev/null)
    ifeq ($(PKG_RAYLIB),)
        LDLIBS += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    else
        CXXFLAGS += $(shell pkg-config --cflags raylib)
        LDLIBS   += $(shell pkg-config --libs raylib)
    endif
endif

.PHONY: all run run-gui run-cli cli clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

run-gui: $(TARGET)
	./$(TARGET)

run-cli: $(TARGET)
	./$(TARGET) --cli

cli: $(TARGET)
	./$(TARGET) --cli

clean:
	rm -rf $(BIN_DIR)
