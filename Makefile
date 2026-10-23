CXX := g++

# Conan-generated pkg-config files live in the project root.
PKG_CONFIG := env PKG_CONFIG_PATH="$(CURDIR):$(PKG_CONFIG_PATH)" pkg-config

IMGUI_BACKENDS_DIR := $(shell $(PKG_CONFIG) --variable=prefix imgui)/res/bindings

CPPFLAGS += $(shell $(PKG_CONFIG) --cflags imgui implot sdl3) -Iinclude -I$(IMGUI_BACKENDS_DIR)
LDLIBS   += $(shell $(PKG_CONFIG) --libs imgui implot sdl3)

#IMGUI_BACKENDS_DIR := $(shell pkg-config --variable=prefix imgui)/res/bindings
IMGUI_PREFIX := $(shell sed -n 's/^prefix=//p' imgui.pc)
IMGUI_BACKENDS_DIR := $(IMGUI_PREFIX)/res/bindings

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
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

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



LOGGER_SRC = logger/logger.cpp
LOGGER_DEPS = src/hours.cpp
LOGGER_OBJ = $(LOGGER_SRC:.cpp=.o) $(LOGGER_DEPS:.cpp=.o)

LOGGER = logger/battery_status_logger_

LDFLAGS = `pkg-config --libs sdl3`

logger: $(LOGGER)

$(LOGGER): $(LOGGER_OBJ)
	$(CXX) -o $@ $(LOGGER_OBJ) $(LDFLAGS)


install_logger: $(LOGGER)
	@echo "Installing executable..."
	sudo cp $(LOGGER) $(PREFIX)/battery_status_logger_
	sudo chmod +x $(PREFIX)/battery_status_logger_
	
	@echo "Creating systemd service..."
	@printf "[Unit]\n\
	Description=Battery Status Logger\n\
	After=network.target\n\n\
	[Service]\n\
	ExecStart=$(PREFIX)/battery_status_logger_\n\
	WorkingDirectory=$(CURRENT_DIR)\n\
	Restart=always\n\
	User=$(CURRENT_USER)\n\n\
	[Install]\n\
	WantedBy=multi-user.target\n" | sudo tee $(SERVICE_PATH)/battery_status_logger.service > /dev/null
	
	@echo "Starting service..."
	sudo systemctl daemon-reload
	sudo systemctl enable battery_status_logger.service
	sudo systemctl start battery_status_logger.service
	@echo "Done."

# Make command to uninstall the logger
uninstall_logger:
	@echo "Stopping and removing service..."
	-sudo systemctl stop battery_status_logger.service
	-sudo systemctl disable battery_status_logger.service
	sudo rm -f $(SERVICE_PATH)/battery_status_logger.service
	sudo systemctl daemon-reload
	@echo "Removing executable..."
	sudo rm -f $(PREFIX)/battery_status_logger_
	@echo "Uninstallation complete."


.PHONY: logger_status
# Check the status of the logger in the system
logger_status:
	@if ! systemctl cat battery_status_logger.service >/dev/null 2>&1; then \
		echo "Logger is not installed"; \
	elif systemctl is-active --quiet battery_status_logger.service; then \
		echo "Logger is active and running"; \
	else \
		echo "Logger is inactive"; \
	fi