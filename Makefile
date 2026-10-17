CXX := g++

# Find Conan's generated .pc files in the project root.
PKG_CONFIG_PATH := $(CURDIR)$(if $(PKG_CONFIG_PATH),:$(PKG_CONFIG_PATH))
export PKG_CONFIG_PATH

CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
#CPPFLAGS += $(shell pkg-config --cflags imgui implot sdl3)
IMGUI_BACKENDS_DIR := $(shell pkg-config --variable=prefix imgui)/res/bindings
CPPFLAGS += $(shell pkg-config --cflags imgui implot sdl3) -I$(IMGUI_BACKENDS_DIR)
LDLIBS   += $(shell pkg-config --libs imgui implot sdl3)

IMGUI_BACKENDS_DIR := $(shell pkg-config --variable=prefix imgui)/res/bindings

TARGET := exec/battery-monitor
BUILD_DIR := build

APP_SOURCES := $(wildcard src/*.cpp)
APP_OBJECTS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(APP_SOURCES))

BACKEND_OBJECTS := \
	$(BUILD_DIR)/imgui_impl_sdl3.o \
	$(BUILD_DIR)/imgui_impl_sdlrenderer3.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(APP_OBJECTS) $(BACKEND_OBJECTS) | exec
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_impl_sdl3.o: $(IMGUI_BACKENDS_DIR)/imgui_impl_sdl3.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_impl_sdlrenderer3.o: $(IMGUI_BACKENDS_DIR)/imgui_impl_sdlrenderer3.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

exec:
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)