# PicoBox Makefile
# BusyBox-style Unix utilities in C
# Now with automatic discovery of refactored commands!

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2 -g
LDFLAGS = -lcurl -L/opt/homebrew/opt/json-c/lib -ljson-c -L/opt/homebrew/opt/argtable3/lib -largtable3
INCLUDES = -Iinclude -I/opt/homebrew/opt/json-c/include -I/opt/homebrew/opt/argtable3/include
BISON = /opt/homebrew/opt/bison/bin/bison
FLEX = flex

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
TEST_DIR = tests
BNFC_DIR = bnfc_shell
COMMANDS_DIR = $(SRC_DIR)/commands
CORE_DIR = $(SRC_DIR)/core

# Target binary
TARGET = $(BUILD_DIR)/picobox

# Automatically discover refactored commands in src/commands/
REFACTORED_CMD_SRCS = $(wildcard $(COMMANDS_DIR)/cmd_*.c)

# Core infrastructure files
CORE_SRCS = $(wildcard $(CORE_DIR)/*.c)

# ALL 25 COMMANDS REFACTORED!
# Refactored: echo, pwd, true, false, basename, dirname, sleep, env, cat, wc, head, tail, touch, mkdir, cp, mv, rm, ln, chmod, stat, df, du, grep, find, ls
LEGACY_CMD_SRCS = \
                  \
                  \
                  \
                  \
                  \
                  $(SRC_DIR)/cmd_ai.c

# Main source files
MAIN_SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/utils.c $(SRC_DIR)/shell.c \
            $(SRC_DIR)/shell_bnfc.c $(SRC_DIR)/exec_helpers.c $(SRC_DIR)/pipe_helpers.c \
            $(SRC_DIR)/redirect_helpers.c $(SRC_DIR)/cmd_compat.c $(SRC_DIR)/var_table.c

# Combine all sources
SRCS = $(MAIN_SRCS) $(LEGACY_CMD_SRCS) $(CORE_SRCS)

# Generate object file paths
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Refactored commands need special compilation (with -DBUILTIN_ONLY)
REFACTORED_CMD_OBJS = $(REFACTORED_CMD_SRCS:$(COMMANDS_DIR)/%.c=$(BUILD_DIR)/refactored_%.o)

# BNFC-generated files
BNFC_SRCS = $(BNFC_DIR)/Absyn.c $(BNFC_DIR)/Buffer.c $(BNFC_DIR)/Printer.c \
            $(BNFC_DIR)/shell_compat.c $(BNFC_DIR)/Shell.tab.c $(BNFC_DIR)/lex.yy.c \
            $(BNFC_DIR)/Skeleton.c
BNFC_OBJS = $(BNFC_SRCS:$(BNFC_DIR)/%.c=$(BUILD_DIR)/bnfc_%.o)

# Installation directory
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

# Commands that will have symlinks
COMMANDS = echo pwd cat mkdir touch ls cp rm mv head tail wc ln \
           grep find basename dirname chmod stat du df env sleep true false

# Default target
.PHONY: all
all: bnfc $(TARGET)

# Create build directory with subdirectories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/core

# Generate BNFC parser files
.PHONY: bnfc
bnfc:
	@cd $(BNFC_DIR) && $(MAKE) --no-print-directory

# Build the main binary (including BNFC objects and refactored commands)
$(TARGET): $(BUILD_DIR) $(OBJS) $(REFACTORED_CMD_OBJS) $(BNFC_OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(REFACTORED_CMD_OBJS) $(BNFC_OBJS)
	@echo "Build complete: $(TARGET)"
	@echo "Refactored commands: $(words $(REFACTORED_CMD_SRCS))"

# Compile main/legacy source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Compile core infrastructure files
$(BUILD_DIR)/core/%.o: $(CORE_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Special rule: Compile refactored commands with -DBUILTIN_ONLY
$(BUILD_DIR)/refactored_%.o: $(COMMANDS_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -DBUILTIN_ONLY -c -o $@ $<

# Compile BNFC-generated files to object files (with relaxed warnings)
$(BUILD_DIR)/bnfc_%.o: $(BNFC_DIR)/%.c
	$(CC) -Wall -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-sign-compare -std=c11 -O2 -g -c -o $@ $<

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	@cd $(BNFC_DIR) && $(MAKE) clean --no-print-directory 2>/dev/null || true
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

# Build standalone binaries for refactored commands (for testing)
.PHONY: standalone
standalone: $(BUILD_DIR)
	@echo "Building standalone binaries for refactored commands..."
	@for cmd_src in $(REFACTORED_CMD_SRCS); do \
		cmd_name=$$(basename $$cmd_src .c | sed 's/cmd_//'); \
		echo "  Building $$cmd_name..."; \
		$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$$cmd_name $$cmd_src $(CORE_DIR)/registry.c $(LDFLAGS); \
	done
	@echo "Standalone binaries created in $(BUILD_DIR)/"

# Show help
.PHONY: help
help:
	@echo "PicoBox Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all         - Build picobox binary (default)"
	@echo "  clean       - Remove build artifacts"
	@echo "  rebuild     - Clean and rebuild"
	@echo "  standalone  - Build standalone binaries for refactored commands"
	@echo "  install     - Install to $(BINDIR) with symlinks"
	@echo "  uninstall   - Remove from $(BINDIR)"
	@echo "  test        - Run test suite"
	@echo "  valgrind    - Run with memory leak detection"
	@echo "  symlinks    - Create local symlinks in build/"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Refactored commands (auto-discovered from $(COMMANDS_DIR)):"
	@for cmd_src in $(REFACTORED_CMD_SRCS); do \
		cmd_name=$$(basename $$cmd_src .c | sed 's/cmd_//'); \
		echo "  - $$cmd_name"; \
	done

# Dependencies
$(BUILD_DIR)/main.o: $(INCLUDE_DIR)/picobox.h $(INCLUDE_DIR)/utils.h
$(BUILD_DIR)/utils.o: $(INCLUDE_DIR)/utils.h
$(BUILD_DIR)/shell_bnfc.o: $(INCLUDE_DIR)/cmd_spec.h
$(BUILD_DIR)/cmd_compat.o: $(INCLUDE_DIR)/cmd_spec.h
$(BUILD_DIR)/core/registry.o: $(INCLUDE_DIR)/cmd_spec.h
$(REFACTORED_CMD_OBJS): $(INCLUDE_DIR)/cmd_spec.h $(INCLUDE_DIR)/picobox.h
