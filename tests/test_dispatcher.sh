#!/bin/bash
#
# Test the picobox dispatcher functionality
#

# Note: Don't use set -e because we're testing error conditions

PICOBOX="./build/picobox"
FAILED=0
PASSED=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "=== Testing PicoBox Dispatcher ==="
echo

# Check if picobox exists
if [ ! -f "$PICOBOX" ]; then
    echo "Error: $PICOBOX not found. Run 'make' first."
    exit 1
fi

# Test function
test_command() {
    local description="$1"
    local command="$2"
    local expected_output="$3"

    echo -n "Testing: $description... "

    if output=$(eval "$command" 2>&1) && echo "$output" | grep -q "$expected_output"; then
        echo -e "${GREEN}PASSED${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAILED${NC}"
        echo "  Expected: $expected_output"
        echo "  Got: $output"
        ((FAILED++))
    fi
}

# Test 1: picobox with no arguments shows help
test_command \
    "picobox with no args shows help" \
    "$PICOBOX" \
    "Available commands"

# Test 2: picobox --help shows help
test_command \
    "picobox --help shows help" \
    "$PICOBOX --help" \
    "Available commands"

# Test 3: Unknown command shows error
test_command \
    "unknown command shows error" \
    "$PICOBOX nonexistent" \
    "unknown command"

# Test 4: Stub commands return "not yet implemented"
test_command \
    "stub echo command" \
    "$PICOBOX echo" \
    "not yet implemented"

test_command \
    "stub pwd command" \
    "$PICOBOX pwd" \
    "not yet implemented"

test_command \
    "stub cat command" \
    "$PICOBOX cat" \
    "not yet implemented"

# Test 5: Version is displayed
test_command \
    "version info in help" \
    "$PICOBOX --help" \
    "PicoBox v"

echo
echo "=== Test Summary ==="
echo "Passed: $PASSED"
echo "Failed: $FAILED"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests PASSED!${NC}"
    exit 0
else
    echo -e "${RED}Some tests FAILED!${NC}"
    exit 1
fi
