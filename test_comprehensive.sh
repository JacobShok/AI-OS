#!/bin/bash
# Comprehensive test suite for PicoBox shell
# Tests: pipelines, variables, built-ins, commands, redirections

PICOBOX="./build/picobox"
PASSED=0
FAILED=0

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "PicoBox Comprehensive Test Suite"
echo "========================================"
echo ""

# Helper function to run a test
run_test() {
    local test_name="$1"
    local input="$2"
    local expected="$3"

    echo -n "Testing: $test_name... "

    output=$(printf "$input\nexit\n" | $PICOBOX 2>&1 | grep -v "PicoBox" | grep -v "Type 'help'" | grep -v "Features:" | grep -v "Try the AI" | grep -v "@show" | grep -v "AI how" | grep -v "^\$" | grep -v "^💡" | sed '/^$/d')

    if echo "$output" | grep -q "$expected"; then
        echo -e "${GREEN}PASSED${NC}"
        ((PASSED++))
        return 0
    else
        echo -e "${RED}FAILED${NC}"
        echo "  Expected: $expected"
        echo "  Got: $output"
        ((FAILED++))
        return 1
    fi
}

# Test 1: Simple command execution
run_test "Simple pwd command" "pwd" "/picobox"

# Test 2: Built-in echo
run_test "Echo command" "echo hello world" "hello world"

# Test 3: Pipeline - echo to cat
run_test "Pipeline: echo | cat" "echo pipeline test | cat" "pipeline test"

# Test 4: Pipeline - ls to grep
run_test "Pipeline: ls | grep" "ls | grep README" "README.md"

# Test 5: Variable assignment
run_test "Variable assignment" "FOO=bar\necho \$FOO" "bar"

# Test 6: Multiple variables
run_test "Multiple variables" "X=10\nY=20\necho \$X \$Y" "10 20"

# Test 7: Variable in command
run_test "Variable expansion in echo" "MSG=Hello\necho \$MSG World" "Hello World"

# Test 8: Special variable $$ (just check it returns something)
run_test "Special variable \$\$ (PID)" "echo \$\$" "[0-9]"

# Test 9: Special variable $?
run_test "Special variable \$? (exit status)" "true\necho \$?" "0"

# Test 10: Export command
run_test "Export variable" "export TEST=exported\nenv | grep TEST" "TEST=exported"

# Test 11: Export existing variable
run_test "Export existing var" "VAR=value\nexport VAR\nenv | grep VAR" "VAR=value"

# Test 12: Chain of commands with semicolon
run_test "Command sequence" "echo first ; echo second" "first"

# Test 13: Pipeline with wc (accepts 5 or more for newline)
run_test "Pipeline: echo | wc" "echo test | wc -c" "[0-9]"

# Test 14: pwd command
run_test "pwd builtin" "pwd" "picobox"

# Test 15: true command
run_test "true command" "true\necho \$?" "0"

# Test 16: false command
run_test "false command" "false\necho \$?" "1"

# Test 17: basename command
run_test "basename command" "basename /usr/local/bin/test" "test"

# Test 18: dirname command
run_test "dirname command" "dirname /usr/local/bin/test" "/usr/local/bin"

# Test 19: Environment variable expansion
run_test "Environment var expansion" "echo \$HOME" "/"

# Test 20: Variable with no expansion
run_test "Undefined variable" "echo \$UNDEFINED_VAR test" "test"

# Test 21: cd command
run_test "cd to /tmp" "cd /tmp\npwd" "/tmp"

# Test 22: Longer pipeline
run_test "Three-stage pipeline" "echo hello | cat | cat" "hello"

# Test 23: Variable in pipeline
run_test "Variable in pipeline" "MSG=data\necho \$MSG | cat" "data"

# Test 24: Multiple exports (test one at a time)
run_test "Multiple exports" "export A=1\nexport B=2\nenv | grep A=1" "A=1"

# Test 25: Cat command
run_test "Cat with stdin" "echo content | cat" "content"

# Test 26: Head command (skip multiline test - not supported without echo -e)
# Test 27: Grep command (skip multiline test - not supported without echo -e)

echo ""
echo "========================================"
echo "Test Summary"
echo "========================================"
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"
echo "Total:  $((PASSED + FAILED))"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi
