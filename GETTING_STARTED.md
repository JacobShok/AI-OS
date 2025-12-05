# PicoBox - Getting Started Guide

Welcome to **PicoBox**, a modern Unix utilities toolkit with an AI-enhanced interactive shell! This guide will help you get up and running quickly.

---

## Table of Contents

1. [What is PicoBox?](#what-is-picobox)
2. [Quick Start](#quick-start)
3. [Installation](#installation)
4. [Using PicoBox](#using-picobox)
5. [Shell Features](#shell-features)
6. [AI Assistants](#ai-assistants)
7. [Package Manager](#package-manager)
8. [Common Tasks](#common-tasks)
9. [Tips & Tricks](#tips--tricks)
10. [Troubleshooting](#troubleshooting)

---

## What is PicoBox?

PicoBox is a BusyBox-style collection of Unix command-line utilities that provides:

✨ **27+ Essential Commands** - All your favorite Unix tools in one binary
🤖 **Dual AI Assistants** - Get help with natural language queries
📦 **Built-in Package Manager** - Install and manage custom tools
🔧 **Modern Shell** - Pipelines, redirections, and smart parsing
🚀 **Single Binary** - One executable, multiple tools via symlinks

**Perfect For:**
- Learning Unix/Linux command-line basics
- Building lightweight embedded systems
- Creating portable toolkits
- Experimenting with shell features

---

## Quick Start

### 1. Build PicoBox

```bash
# Clone or navigate to the picobox directory
cd picobox

# Build the project
make clean && make

# The binary will be at: ./build/picobox
```

### 2. Run Your First Command

```bash
# Display text
./build/picobox echo "Hello, PicoBox!"

# List files
./build/picobox ls

# Show current directory
./build/picobox pwd
```

### 3. Start Interactive Shell

```bash
# Start the advanced BNFC-powered shell
PICOBOX_BNFC=1 ./build/picobox

# You should see:
# PicoBox BNFC Shell v0.1.0 (Visitor Pattern + Registry + AI)
# Type 'help' for available commands, 'exit' to quit.
# $
```

### 4. Try AI Features

```bash
# In the shell, ask the AI for help:
$ @show me all files
💡 AI Suggested Command:
   ls -la
Run this command? (y/n): y

# Or use the legacy AI command:
$ AI how do I count lines in a file
✨ Use: wc -l filename
```

---

## Installation

### System Requirements

**Operating System:**
- macOS (tested on Darwin 24.3.0)
- Linux (Ubuntu, Debian, Fedora, etc.)
- BSD systems (likely compatible)

**Build Dependencies:**
```bash
# macOS (Homebrew)
brew install gcc make bison flex argtable3 json-c curl

# Ubuntu/Debian
sudo apt install build-essential bison flex libargtable3-dev libjson-c-dev libcurl4-openssl-dev

# Fedora
sudo dnf install gcc make bison flex argtable3-devel json-c-devel libcurl-devel
```

**Runtime Requirements:**
- POSIX-compatible operating system
- C11 standard library
- Python 3.6+ (for AI assistant features)

### Building from Source

```bash
# 1. Clone the repository (if from git)
git clone <repository-url>
cd picobox

# 2. Build all components
make

# 3. Test the build
./build/picobox --help

# 4. Run tests (optional)
make test
```

### System Installation

```bash
# Install to /usr/local/bin (requires sudo)
sudo make install

# This creates:
# - /usr/local/bin/picobox (main binary)
# - Symlinks for all commands (ls, cat, echo, etc.)

# Now you can use commands directly:
ls --help
echo "Installed successfully!"

# Or use the shell:
picobox
```

### Optional: AI Features Setup

To enable AI assistant features, you need an OpenAI API key:

```bash
# Get your API key from: https://platform.openai.com/api-keys

# Set environment variable (add to ~/.bashrc or ~/.zshrc)
export AI_SHELL="sk-your-api-key-here"
# or
export OPENAI_API_KEY="sk-your-api-key-here"

# For AI helper Python script:
pip3 install openai  # Optional, but recommended
```

---

## Using PicoBox

### Command Mode

Run individual commands directly:

```bash
# Syntax: picobox <command> [arguments...]
./build/picobox echo "Hello World"
./build/picobox ls -la
./build/picobox cat file.txt
./build/picobox grep "pattern" file.txt
```

### Symlink Mode

If installed system-wide, commands work as standalone programs:

```bash
# These are actually symlinks to picobox:
ls -l
cat README.md
grep "TODO" *.c
```

### Shell Mode

Interactive shell for running multiple commands:

```bash
# Start simple shell
./build/picobox

# Start advanced BNFC shell (recommended)
PICOBOX_BNFC=1 ./build/picobox

# Or add to your shell config:
alias mysh='PICOBOX_BNFC=1 /path/to/build/picobox'
```

---

## Shell Features

### Basic Commands

```bash
$ pwd
/Users/username/picobox

$ ls
Makefile  README.md  build/  src/  include/

$ echo "Testing PicoBox"
Testing PicoBox

$ cat README.md
# PicoBox
...
```

### Pipelines

Connect commands together - output of one becomes input of next:

```bash
# Count lines in all .c files
$ ls | grep ".c" | wc -l

# Find and sort unique words
$ cat file.txt | tr ' ' '\n' | sort | uniq

# Search for errors in log files
$ cat error.log | grep "ERROR" | head -10
```

### Input/Output Redirection

```bash
# Read from file
$ cat < input.txt

# Write to file (overwrites)
$ echo "Hello" > output.txt

# Append to file
$ echo "World" >> output.txt

# Combine redirections
$ grep "error" < input.log > errors.txt
```

### Command Sequences

Run multiple commands in sequence:

```bash
# Create directory and file, then list
$ mkdir testdir ; touch testdir/file.txt ; ls testdir

# Multiple operations in one line
$ echo "Line 1" > file.txt ; echo "Line 2" >> file.txt ; cat file.txt
```

### Built-in Commands

Special commands that run in the shell process:

```bash
# Change directory
$ cd /tmp
$ pwd
/tmp

# Display help
$ help

# Exit shell
$ exit
```

---

## AI Assistants

PicoBox includes **two AI systems** to help you:

### 1. Natural Language Command Suggestions (`@` prefix)

Ask in plain English, get executable commands:

```bash
$ @show me all files including hidden ones
💡 AI Suggested Command:
   ls -la
Run this command? (y/n): y
# Output appears here

$ @find all python files
💡 AI Suggested Command:
   find . -name "*.py"
Run this command? (y/n): n
Command cancelled.

$ @count lines in all text files
💡 AI Suggested Command:
   cat *.txt | wc -l
Run this command? (y/n): y
42
```

**How it works:**
1. Types `@` followed by natural language query
2. AI analyzes available commands (RAG)
3. Generates suggested command
4. Asks for confirmation (y/n)
5. Executes if approved

**Features:**
- Works without API key (uses heuristics)
- Better with OpenAI API (smart suggestions)
- Safe - always asks for confirmation
- Learns from available commands

### 2. AI Command Help (`AI` command)

Ask questions about commands and concepts:

```bash
$ AI how do I list all files
✨ Use: ls -la

$ AI what does grep do
✨ grep searches for text patterns in files.
   Use: grep 'pattern' filename

$ AI how do I count words in a file
✨ Use: wc -w filename

$ AI explain the pipe operator
✨ The pipe (|) connects commands, sending output
   from one command as input to the next.
```

**Features:**
- Conversational responses
- Explains concepts
- Provides command examples
- Requires OpenAI API key

### Comparison

| Feature | `@query` | `AI command` |
|---------|----------|--------------|
| Usage | `@<natural language>` | `AI <question>` |
| Output | Single command | Explanation |
| Execution | Auto (with confirmation) | Manual |
| API Required | No (optional) | Yes |
| Best For | Quick tasks | Learning |

---

## Package Manager

Install and manage additional tools with the built-in package manager.

### Setup

Add `~/.mysh/bin` to your PATH:

```bash
# Add to ~/.bashrc or ~/.zshrc
export PATH="$HOME/.mysh/bin:$PATH"
```

### Installing Packages

```bash
# Install a package from .tar.gz file
$ pkg install hello-1.0.0.tar.gz
Extracting package...
Package: hello version 1.0.0
Description: Hello world program
Installing to /Users/user/.mysh/packages/hello-1.0.0...
Creating symlinks for binaries:
  hello -> /Users/user/.mysh/packages/hello-1.0.0/hello
Package 'hello' installed successfully!
```

### Listing Packages

```bash
$ pkg list
Installed packages:
NAME                 VERSION      DESCRIPTION
----                 -------      -----------
hello                1.0.0        Hello world program

Total: 1 package
```

### Package Information

```bash
$ pkg info hello
Package: hello
Version: 1.0.0
Description: Hello world program
Installed: 2025-12-05
Location: /Users/user/.mysh/packages/hello-1.0.0

Files:
  hello
  pkg.json
```

### Removing Packages

```bash
$ pkg remove hello
Removing package 'hello'...
Package 'hello' removed successfully
```

### Creating Packages

Package format: `.tar.gz` archive with `pkg.json`

**Example pkg.json:**
```json
{
  "name": "mytool",
  "version": "1.0.0",
  "description": "My custom tool",
  "binaries": ["mytool"]
}
```

**Create package:**
```bash
# 1. Create directory structure
mkdir mytool
cd mytool

# 2. Add your binary and pkg.json
cp /path/to/mytool .
cat > pkg.json << EOF
{
  "name": "mytool",
  "version": "1.0.0",
  "description": "My custom tool",
  "binaries": ["mytool"]
}
EOF

# 3. Create tarball
cd ..
tar -czf mytool-1.0.0.tar.gz mytool/

# 4. Install
pkg install mytool-1.0.0.tar.gz
```

---

## Common Tasks

### File Operations

```bash
# Create empty file
$ touch newfile.txt

# Create directory
$ mkdir mydir

# Copy file
$ cp source.txt destination.txt

# Move/rename file
$ mv oldname.txt newname.txt

# Remove file
$ rm unwanted.txt

# Create symbolic link
$ ln -s /path/to/target linkname
```

### Viewing Files

```bash
# Display entire file
$ cat file.txt

# First 10 lines
$ head file.txt

# Last 10 lines
$ tail file.txt

# First 5 lines
$ head -n 5 file.txt

# Count lines, words, characters
$ wc file.txt
```

### Searching

```bash
# Search for pattern in file
$ grep "error" logfile.txt

# Case-insensitive search
$ grep -i "ERROR" logfile.txt

# Search in multiple files
$ grep "TODO" *.c

# Find files by name
$ find . -name "*.txt"

# Find files modified in last day
$ find . -type f -mtime -1
```

### System Information

```bash
# Show environment variables
$ env

# Show disk usage
$ df -h

# Show directory size
$ du -sh /path/to/dir

# File statistics
$ stat file.txt

# Current directory
$ pwd
```

### Text Processing

```bash
# Count lines in multiple files
$ cat *.txt | wc -l

# Search and count matches
$ grep "pattern" file.txt | wc -l

# Extract filenames from paths
$ echo "/usr/local/bin/program" | basename

# Extract directory from path
$ echo "/usr/local/bin/program" | dirname

# Display without newline
$ echo -n "No newline"
```

---

## Tips & Tricks

### Productivity Tips

**1. Use Tab Completion (if your terminal supports it)**
```bash
$ cat READ<TAB>  # Completes to README.md
```

**2. Use AI for Complex Commands**
```bash
$ @find all files larger than 1MB modified in the last week
# AI figures out the command for you!
```

**3. Chain Commands**
```bash
# Create directory and enter it
$ mkdir project ; cd project ; pwd

# Backup and verify
$ cp important.txt important.txt.bak ; ls -l important.txt*
```

**4. Combine Pipes and Redirections**
```bash
# Process and save
$ cat input.txt | grep "data" | sort > output.txt

# Count and save
$ ls | wc -l > file_count.txt
```

### Shell Shortcuts

```bash
# Ctrl+D - Exit shell (EOF)
# Ctrl+C - Cancel current command
# Ctrl+L - Clear screen (if supported by terminal)
```

### Advanced Patterns

**Process Multiple Files:**
```bash
# Count total lines in all .txt files
$ cat *.txt | wc -l

# Find pattern in all files, show filenames
$ grep -n "TODO" src/*.c
```

**Data Processing:**
```bash
# Extract unique values
$ cat data.txt | sort | uniq

# Count frequency
$ cat data.txt | sort | uniq -c

# Top 10 most common lines
$ cat data.txt | sort | uniq -c | sort -rn | head -10
```

**Log Analysis:**
```bash
# Count errors by type
$ grep "ERROR" app.log | cut -d: -f2 | sort | uniq -c

# Recent errors
$ tail -100 app.log | grep "ERROR"
```

---

## Troubleshooting

### Build Issues

**Problem:** `command not found: bison` or `command not found: flex`
```bash
# Solution: Install dependencies
# macOS:
brew install bison flex

# Linux:
sudo apt install bison flex
```

**Problem:** `argtable3.h: No such file or directory`
```bash
# Solution: Install argtable3
# macOS:
brew install argtable3

# Linux:
sudo apt install libargtable3-dev
```

**Problem:** Build fails with linker errors
```bash
# Solution: Check library paths in Makefile
# Edit Makefile and adjust LDFLAGS and INCLUDES
# for your system's library locations
```

### Runtime Issues

**Problem:** AI features don't work
```bash
# Solution: Set API key
export AI_SHELL="sk-your-api-key-here"

# Or check if key is set:
echo $AI_SHELL

# Test AI manually:
./build/picobox
$ AI test
```

**Problem:** `@query` returns "No suggestion"
```bash
# Solution 1: Check if Python script exists
ls mysh_llm.py

# Solution 2: Try running manually
python3 mysh_llm.py "list files"

# Solution 3: Set debug mode
export MYSH_LLM_DEBUG=1
```

**Problem:** Commands not found in shell
```bash
# Solution: Make sure you're using BNFC shell
PICOBOX_BNFC=1 ./build/picobox

# Check if command is registered:
./build/picobox --help
```

### Permission Issues

**Problem:** `Permission denied` when running commands
```bash
# Solution: Check file permissions
ls -l build/picobox
chmod +x build/picobox
```

**Problem:** `Permission denied` during installation
```bash
# Solution: Use sudo for system installation
sudo make install

# Or install to user directory:
PREFIX=$HOME/.local make install
export PATH="$HOME/.local/bin:$PATH"
```

### Package Manager Issues

**Problem:** `pkg: HOME environment variable not set`
```bash
# Solution: Set HOME
export HOME=$HOME  # Should already be set
echo $HOME
```

**Problem:** Installed packages not in PATH
```bash
# Solution: Add to PATH
export PATH="$HOME/.mysh/bin:$PATH"

# Add to shell config (~/.bashrc or ~/.zshrc):
echo 'export PATH="$HOME/.mysh/bin:$PATH"' >> ~/.bashrc
```

### Getting Help

**In-Shell Help:**
```bash
$ help                    # Shell help
$ <command> --help        # Command-specific help
$ AI what is picobox      # Ask the AI
```

**System Help:**
```bash
# Show all available commands
./build/picobox --help

# Export commands as JSON (for AI)
./build/picobox --commands-json

# Check version
./build/picobox --help | head -1
```

**Debugging:**
```bash
# Enable debug mode for AI
export MYSH_LLM_DEBUG=1

# Run with verbose output
./build/picobox <command> -v  # If supported

# Check memory leaks
make valgrind
```

---

## Next Steps

### Learn More

1. **Read the Technical Documentation**
   - See [DOCUMENTATION.md](DOCUMENTATION.md) for architecture details
   - Understand how commands are implemented
   - Learn about the visitor pattern and BNFC parser

2. **Explore Commands**
   - Try each command with `--help`
   - Experiment with pipelines
   - Combine commands creatively

3. **Use AI Features**
   - Practice with `@` queries
   - Learn command patterns
   - Ask `AI` for explanations

4. **Build Packages**
   - Create your own tools
   - Package them with `pkg`
   - Share with others

### Advanced Topics

**Custom Commands:**
- Study `src/commands/cmd_echo.c` as template
- Add your own commands
- Rebuild with `make`

**Shell Scripting:**
- Save command sequences to files
- Use command-line arguments
- Build complex workflows

**System Integration:**
- Add picobox to system PATH
- Create aliases in shell config
- Integrate with other tools

---

## Quick Reference Card

### Essential Commands
```bash
# Files
ls          # List files
cat         # Display file
cp          # Copy
mv          # Move/rename
rm          # Remove
touch       # Create/update file
mkdir       # Create directory

# Search
grep        # Search in files
find        # Find files

# Text
head        # First lines
tail        # Last lines
wc          # Count lines/words
echo        # Print text

# System
pwd         # Current directory
env         # Environment
df          # Disk space
du          # Directory size

# Shell
cd          # Change directory
help        # Show help
exit        # Exit shell
```

### Shell Operators
```bash
|           # Pipe (connect commands)
>           # Redirect output (overwrite)
>>          # Redirect output (append)
<           # Redirect input
;           # Command separator
```

### AI Commands
```bash
@<query>    # Natural language command
AI <question>  # Ask for help
```

### Package Manager
```bash
pkg install <file>   # Install package
pkg list             # List packages
pkg info <name>      # Show info
pkg remove <name>    # Remove package
```

---

## Getting Support

### Resources

- **Documentation:** [DOCUMENTATION.md](DOCUMENTATION.md)
- **README:** [README.md](README.md)
- **Source Code:** Check inline comments
- **Tests:** See `tests/` directory for examples

### Common Questions

**Q: Is PicoBox a replacement for Bash?**
A: No, it's a learning tool and lightweight shell. Use Bash/Zsh for production.

**Q: Can I use PicoBox commands in Bash scripts?**
A: Yes, if installed system-wide or in PATH.

**Q: Are AI features required?**
A: No, all core features work without AI. AI is optional enhancement.

**Q: How much does OpenAI API cost?**
A: Minimal - typical queries cost $0.001-0.01. Check OpenAI pricing.

**Q: Can I contribute new commands?**
A: Yes! Follow the command anatomy in existing commands.

---

## Conclusion

You're now ready to use PicoBox! Key takeaways:

✅ **Build and run** with `make` and `./build/picobox`
✅ **Use commands** directly or via interactive shell
✅ **Leverage AI** with `@queries` and `AI command`
✅ **Manage packages** with built-in package manager
✅ **Combine features** using pipes and redirections

**Start exploring:**
```bash
# Build
make

# Enter shell
PICOBOX_BNFC=1 ./build/picobox

# Try AI
$ @show me all files
$ AI how do I search for text

# Have fun!
$ echo "Happy hacking with PicoBox!" | cat
```

For detailed technical information, see [DOCUMENTATION.md](DOCUMENTATION.md).

---

**Welcome to PicoBox - Unix utilities reimagined! 🚀**
