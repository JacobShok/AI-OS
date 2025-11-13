# PicoBox Makefile
# BusyBox-style Unix utilities in C

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2 -g
LDFLAGS =
INCLUDES = -Iinclude

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
TEST_DIR = tests

# Target binary
TARGET = $(BUILD_DIR)/picobox

# Source files
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/utils.c $(SRC_DIR)/shell.c \
       $(SRC_DIR)/cmd_echo.c $(SRC_DIR)/cmd_pwd.c $(SRC_DIR)/cmd_cat.c \
       $(SRC_DIR)/cmd_mkdir.c $(SRC_DIR)/cmd_touch.c $(SRC_DIR)/cmd_ls.c \
       $(SRC_DIR)/cmd_cp.c $(SRC_DIR)/cmd_rm.c $(SRC_DIR)/cmd_mv.c \
       $(SRC_DIR)/cmd_head.c $(SRC_DIR)/cmd_tail.c $(SRC_DIR)/cmd_wc.c \
       $(SRC_DIR)/cmd_ln.c $(SRC_DIR)/cmd_grep.c $(SRC_DIR)/cmd_find.c \
       $(SRC_DIR)/cmd_basename.c $(SRC_DIR)/cmd_dirname.c $(SRC_DIR)/cmd_chmod.c \
       $(SRC_DIR)/cmd_stat.c $(SRC_DIR)/cmd_du.c $(SRC_DIR)/cmd_df.c \
       $(SRC_DIR)/cmd_env.c $(SRC_DIR)/cmd_sleep.c $(SRC_DIR)/cmd_true.c \
       $(SRC_DIR)/cmd_false.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Installation directory
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

# Commands that will have symlinks
COMMANDS = echo pwd cat mkdir touch ls cp rm mv head tail wc ln \
           grep find basename dirname chmod stat du df env sleep true false

# Default target
.PHONY: all
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the main binary
$(TARGET): $(BUILD_DIR) $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)
	@echo "Build complete: $(TARGET)"

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	@echo "Build artifacts cleaned"

# Install binary and create symlinks
.PHONY: install
install: $(TARGET)
	@echo "Installing picobox to $(BINDIR)..."
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/picobox
	@echo "Creating symlinks for commands..."
	@for cmd in $(COMMANDS); do \
		ln -sf picobox $(BINDIR)/$$cmd; \
		echo "  Created symlink: $(BINDIR)/$$cmd -> picobox"; \
	done
	@echo "Installation complete"

# Uninstall binary and remove symlinks
.PHONY: uninstall
uninstall:
	@echo "Removing picobox from $(BINDIR)..."
	rm -f $(BINDIR)/picobox
	@echo "Removing command symlinks..."
	@for cmd in $(COMMANDS); do \
		rm -f $(BINDIR)/$$cmd; \
		echo "  Removed: $(BINDIR)/$$cmd"; \
	done
	@echo "Uninstallation complete"

# Run tests (skeleton for now)
.PHONY: test
test: $(TARGET)
	@echo "Running tests..."
	@if [ -d $(TEST_DIR) ]; then \
		for test in $(TEST_DIR)/test_*.sh; do \
			if [ -f "$$test" ]; then \
				echo "Running $$test..."; \
				bash "$$test"; \
			fi; \
		done; \
	else \
		echo "No tests directory found"; \
	fi

# Run with valgrind for memory leak detection
.PHONY: valgrind
valgrind: $(TARGET)
	@echo "Running valgrind memory check..."
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		$(TARGET) --help

# Create local symlinks for testing (in build directory)
.PHONY: symlinks
symlinks: $(TARGET)
	@echo "Creating local symlinks in $(BUILD_DIR)..."
	@for cmd in $(COMMANDS); do \
		ln -sf picobox $(BUILD_DIR)/$$cmd; \
	done
	@echo "Local symlinks created"

# Rebuild everything from scratch
.PHONY: rebuild
rebuild: clean all

# Show help
.PHONY: help
help:
	@echo "PicoBox Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build picobox binary (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install to $(BINDIR) with symlinks"
	@echo "  uninstall  - Remove from $(BINDIR)"
	@echo "  test       - Run test suite"
	@echo "  valgrind   - Run with memory leak detection"
	@echo "  symlinks   - Create local symlinks in build/"
	@echo "  rebuild    - Clean and rebuild"
	@echo "  help       - Show this help message"

# Dependencies
$(BUILD_DIR)/main.o: $(INCLUDE_DIR)/picobox.h $(INCLUDE_DIR)/utils.h
$(BUILD_DIR)/utils.o: $(INCLUDE_DIR)/utils.h
