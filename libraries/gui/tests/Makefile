# =============================================================================
# NusaCore GUI Framework — Test Makefile
# Runs on host Linux (x86/x64).  No SDL2, no LVGL, no hardware required.
#
# Usage:
#   cd app/tests
#   make          — build and run all tests
#   make clean    — remove test binaries
# =============================================================================

CC     = gcc
BUILD_DIR = build

CFLAGS = -Wall -Wextra -g -O0 \
         -I..            \
         -I../src

# gesture.c is pure C with no dependencies other than gesture.h
# test_gesture.c provides gesture_tick_ms() (simulated clock)
GESTURE_SRCS = test_gesture.c ../src/gesture.c

# Target path inside build directory
TARGET = $(BUILD_DIR)/test_gesture

.PHONY: all clean run

all: $(TARGET) run
	@echo ""
	@echo "All tests complete."

# Rule to create the build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build target: depends on sources (normal) and build dir (order-only)
$(TARGET): $(GESTURE_SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET)
	@echo ""
	@echo "Running gesture tests..."
	@echo "------------------------"
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
