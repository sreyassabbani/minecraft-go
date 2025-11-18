CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -I.

# Host harness ---------------------------------------------------------------
HOST_BIN := host/host_test
HOST_SRC := host/main.cpp utils/general.cpp utils/algebra.cpp

# Microcontroller build/upload ----------------------------------------------
ARDUINO_CLI ?= arduino-cli

# Default to Arduino Due (programming port). Override if you use a different board.
MCU_FQBN ?= arduino:sam:arduino_due_x
MCU_PORT ?=
SKETCH := minecraft-go.ino
MCU_BUILD_DIR := build/mcu

.PHONY: all host host-test host-run mcu-build mcu-upload mcu-ports clean

all: host

host: $(HOST_BIN)

host-test: host-run

host-run: host
	./$(HOST_BIN)

$(HOST_BIN): $(HOST_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(HOST_SRC) -o $@

mcu-build: guard-arduino-cli
	@mkdir -p $(MCU_BUILD_DIR)
	$(ARDUINO_CLI) compile --fqbn $(MCU_FQBN) --output-dir $(MCU_BUILD_DIR) $(SKETCH)

mcu-upload: guard-arduino-cli guard-mcu-port
	$(ARDUINO_CLI) upload --fqbn $(MCU_FQBN) -p $(MCU_PORT) $(SKETCH)

mcu-ports:
	@if command -v $(ARDUINO_CLI) >/dev/null 2>&1; then \
		echo "arduino-cli board list"; \
		$(ARDUINO_CLI) board list; \
	else \
		if [ "$$OS" = "Windows_NT" ]; then \
			echo "arduino-cli not found; attempting Windows serial port discovery (COM*):"; \
			powershell -NoProfile -Command "Get-CimInstance Win32_SerialPort | Select-Object -ExpandProperty DeviceID" 2>/dev/null || true; \
		else \
			echo "arduino-cli not found; listing common serial devices instead:"; \
			ls /dev/tty.* /dev/cu.* 2>/dev/null || true; \
		fi; \
	fi

guard-arduino-cli:
	@command -v $(ARDUINO_CLI) >/dev/null 2>&1 || (echo "arduino-cli not found. Install it or point ARDUINO_CLI to the binary." && exit 1)

guard-mcu-port:
	@if [ -z "$(MCU_PORT)" ]; then \
		echo "MCU_PORT is not set. Use \`make mcu-ports\` to discover ports, then set MCU_PORT to your COM port (Windows) or /dev/tty.* device (macOS/Linux)."; \
		exit 1; \
	fi

clean:
	rm -f $(HOST_BIN)
	rm -rf $(MCU_BUILD_DIR)
