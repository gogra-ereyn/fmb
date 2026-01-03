BUILD_DIR ?= build
CONFIG    ?= RelWithDebugInfo
JOBS      ?= 4
ARGS      ?=
CMAKE     ?= cmake
CTEST     ?= ctest

.PHONY: all build debug release test clean rebuild run run-%

all: build

build:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CONFIG) $(ARGS)
	$(CMAKE) --build $(BUILD_DIR) -- -j $(JOBS)

debug:
	$(MAKE) build CONFIG=Debug

release:
	$(MAKE) build CONFIG=Release

test: debug
	$(CTEST) --output-on-failure --test-dir $(BUILD_DIR)

clean:
	@echo "Removing $(BUILD_DIR)/"
	rm -rf $(BUILD_DIR)

rebuild: clean build

run: build
	@if [ ! -x "$(BIN_PATH)" ]; then \
	    echo "❌  Executable '$(BIN_PATH)' not found. Did you add_executable($(BIN)) ?" >&2 ; \
	    exit 1 ; \
	fi
	@echo "▶️  Running $(BIN_PATH) $(ARGS)"
	@$(BIN_PATH) $(ARGS)


run-%: ; $(MAKE) run BIN=$* ARGS="$(ARGS)"

