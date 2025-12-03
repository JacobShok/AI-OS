# Makefile for BNFC-generated Shell parser
# Generated for macOS with Homebrew bison

CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
BISON = /opt/homebrew/opt/bison/bin/bison
FLEX = flex

# Generated files
GENERATED = Shell.tab.c Bison.h lex.yy.c
OBJECTS = Shell.tab.o lex.yy.o Absyn.o Buffer.o Printer.o shell_compat.o

# Target
TARGET = TestShell

all: $(TARGET)

# Generate parser and lexer
Shell.tab.c Bison.h: Shell.y
	$(BISON) -d Shell.y

lex.yy.c: Shell.l Bison.h
	$(FLEX) Shell.l

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link test program
$(TARGET): $(OBJECTS) Test.c
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) Test.c

# Clean
clean:
	rm -f $(OBJECTS) $(GENERATED) $(TARGET)

# Rebuild from scratch
rebuild: clean all

.PHONY: all clean rebuild
