# Simple developer entry points for io-core.

CMAKE ?= cmake
CTEST ?= ctest
BUILD_DIR ?= build
BUILD_TYPE ?= Debug
GENERATOR ?=

CMAKE_CONFIGURE_ARGS := -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
ifneq ($(strip $(GENERATOR)),)
CMAKE_CONFIGURE_ARGS += -G "$(GENERATOR)"
endif

.PHONY: help configure build test format format-check tidy clean rebuild install

help:
	@echo "Available targets:"
	@echo "  make configure    - Configure CMake in $(BUILD_DIR)"
	@echo "  make build        - Configure and build library + tests"
	@echo "  make test         - Build and run CTest suite"
	@echo "  make format       - Apply clang-format to project sources"
	@echo "  make format-check - Verify clang-format compliance"
	@echo "  make tidy         - Run clang-tidy static analysis"
	@echo "  make install      - Install from $(BUILD_DIR)"
	@echo "  make clean        - Remove $(BUILD_DIR)"
	@echo "  make rebuild      - Clean then build"
	@echo "Variables: BUILD_DIR, BUILD_TYPE, GENERATOR"

configure:
	$(CMAKE) $(CMAKE_CONFIGURE_ARGS)

build: configure
	$(CMAKE) --build $(BUILD_DIR) -j

test: build
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

lint: configure
	$(CMAKE) --build $(BUILD_DIR) --target format

format: configure
	$(CMAKE) --build $(BUILD_DIR) --target format

format-check: configure
	$(CMAKE) --build $(BUILD_DIR) --target format-check

tidy: configure
	$(CMAKE) --build $(BUILD_DIR) --target tidy

install: build
	$(CMAKE) --install $(BUILD_DIR)

clean:
	$(CMAKE) -E rm -rf $(BUILD_DIR)

rebuild: clean build
