#!/bin/bash
#
# Comprehensive Test Suite for PicoBox Threading & Variables
# Tests all features implemented during the session
#

set -e  # Exit on first failure

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

PICOBOX="./build/picobox"
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test
run_test() {
    local test_name="$1"
    local expected="$2"
    local input="$3"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    echo -e "${BLUE}[TEST $TOTAL_TESTS]${NC} $test_name"

    # Run the test
    local output=$(PICOBOX_BNFC=1 $PICOBOX <<EOF
$input
exit
EOF
)

    # Extract just the command output (skip shell prompt lines)
    # 1. Remove banner lines
    # 2. Remove lines that are ONLY "$" (prompts with no output)
    # 3. For lines starting with "$ ", remove the "$ " prefix to get actual output
    # 4. Remove remaining empty lines
    local actual=$(echo "$output" | \
        grep -v "^PicoBox" | \
        grep -v "^Type" | \
        grep -v "^Features:" | \
        grep -v "^💡" | \
        grep -v "^   @" | \
        grep -v "^   AI" | \
        grep -v "^$" | \
        sed 's/^\$ //' | \
        grep -v "^$")

    if echo "$actual" | grep -q "$expected"; then
        echo -e "${GREEN}✓ PASS${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo -e "${RED}✗ FAIL${NC}"
        echo "  Expected: $expected"
        echo "  Got:      $actual"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

# Start tests
echo -e "${YELLOW}========================================${NC}"
echo -e "${YELLOW}  PicoBox Feature Test Suite${NC}"
echo -e "${YELLOW}========================================${NC}"
echo ""

# ============================================================================
echo -e "${YELLOW}[CATEGORY 1] Shell Variables - Basic${NC}"
# ============================================================================

run_test "Variable assignment and expansion" \
    "hello" \
    "FOO=hello
echo \$FOO"

run_test "Variable with multiple words (no quotes)" \
    "hello world" \
    "MSG=hello
MSG2=world
echo \$MSG \$MSG2"

run_test "Overwriting variables" \
    "world" \
    "FOO=hello
FOO=world
echo \$FOO"

run_test "Empty variable expands to nothing" \
    "before  after" \
    "EMPTY=
echo before \$EMPTY after"

run_test "Undefined variable expands to nothing" \
    "before  after" \
    "echo before \$NOTDEFINED after"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 2] Special Variables${NC}"
# ============================================================================

run_test "Exit status \$? after successful command" \
    "0" \
    "true
echo \$?"

run_test "Exit status \$? after failed command" \
    "1" \
    "false
echo \$?"

run_test "Process ID \$\$ is numeric" \
    "[0-9]" \
    "echo \$\$"

run_test "Shell name \$0" \
    "picobox" \
    "echo \$0"

run_test "Multiple \$? checks" \
    "0" \
    "true
echo \$?
false
echo \$?
true
echo \$?"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 3] Export Functionality${NC}"
# ============================================================================

run_test "Export with VALUE" \
    "testvalue" \
    "export TESTVAR=testvalue
env | grep TESTVAR"

run_test "Export existing variable" \
    "exported" \
    "MYVAR=exported
export MYVAR
env | grep MYVAR"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 4] Variable Expansion in Pipelines${NC}"
# ============================================================================

run_test "Variable expansion before pipeline" \
    "8" \
    "NAME=picobox
echo \$NAME | wc -c"

run_test "Variable in middle of pipeline" \
    "test" \
    "VAR=test
echo \$VAR | cat"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 5] Basic Pipelines (Built-ins Only)${NC}"
# ============================================================================

run_test "Two-stage pipeline: echo | wc" \
    "6" \
    "echo hello | wc -c"

run_test "Three-stage pipeline: echo | cat | wc" \
    "6" \
    "echo hello | cat | wc -c"

run_test "Pipeline with word count" \
    "3" \
    "echo hello world test | wc -w"

run_test "Pipeline with line count" \
    "1" \
    "echo hello | wc -l"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 6] Mixed Pipelines (Built-in + External)${NC}"
# ============================================================================

run_test "Built-in to external: echo | /bin/cat" \
    "mixed" \
    "echo mixed | /bin/cat"

run_test "External to built-in: /bin/echo | wc" \
    "6" \
    "/bin/echo hello | wc -c"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 7] Command Exit Status Propagation${NC}"
# ============================================================================

run_test "Pipeline exit status from last command (success)" \
    "0" \
    "true | true | true
echo \$?"

run_test "Pipeline exit status from last command (failure)" \
    "1" \
    "true | true | false
echo \$?"

run_test "Simple command exit status (true)" \
    "0" \
    "true
echo \$?"

run_test "Simple command exit status (false)" \
    "1" \
    "false
echo \$?"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 8] Built-in Command Functionality${NC}"
# ============================================================================

run_test "Echo with multiple arguments" \
    "one two three" \
    "echo one two three"

run_test "Echo with -n flag (no newline)" \
    "nonewline" \
    "echo -n nonewline"

run_test "Cat from stdin" \
    "cattest" \
    "echo cattest | cat"

run_test "Wc byte count" \
    "11" \
    "echo hello world | wc -c"

run_test "Wc word count" \
    "2" \
    "echo hello world | wc -w"

run_test "Wc line count" \
    "1" \
    "echo test | wc -l"

run_test "Basename basic" \
    "file.txt" \
    "basename /path/to/file.txt"

run_test "Dirname basic" \
    "/path/to" \
    "dirname /path/to/file.txt"

run_test "Pwd returns a path" \
    "/" \
    "pwd"

run_test "True returns 0" \
    "0" \
    "true
echo \$?"

run_test "False returns 1" \
    "1" \
    "false
echo \$?"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 9] Help Output (Stream Redirection)${NC}"
# ============================================================================

run_test "Echo help works" \
    "Usage: echo" \
    "echo --help | head -1"

run_test "Cat help works" \
    "Usage: cat" \
    "cat --help | head -1"

run_test "True help works" \
    "Usage: true" \
    "true --help | head -1"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 10] Complex Variable Scenarios${NC}"
# ============================================================================

run_test "Variable with special chars in value" \
    "hello-world_test.txt" \
    "FILE=hello-world_test.txt
echo \$FILE"

run_test "Variable with path" \
    "/usr/local/bin" \
    "PATH_VAR=/usr/local/bin
echo \$PATH_VAR"

run_test "Multiple variables in one echo" \
    "hello world" \
    "A=hello
B=world
echo \$A \$B"

run_test "Variable with number" \
    "test123" \
    "VAR=test123
echo \$VAR"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 11] Edge Cases${NC}"
# ============================================================================

run_test "Empty command line (just enter)" \
    "" \
    "
"

run_test "Multiple semicolons" \
    "first second" \
    "echo first ; echo second"

run_test "Variable assignment without echo" \
    "" \
    "SILENT=value"

run_test "Echo with no arguments" \
    "^$" \
    "echo"

# ============================================================================
echo ""
echo -e "${YELLOW}[CATEGORY 12] Grammar Features${NC}"
# ============================================================================

run_test "Dollar sign in \$? works" \
    "0" \
    "true
echo \$?"

run_test "Equals sign in variable assignment" \
    "test" \
    "VAR=test
echo \$VAR"

run_test "Colon in path variable" \
    "/bin:/usr/bin" \
    "MYPATH=/bin:/usr/bin
echo \$MYPATH"

# ============================================================================
# Print summary
# ============================================================================

echo ""
echo -e "${YELLOW}========================================${NC}"
echo -e "${YELLOW}  Test Summary${NC}"
echo -e "${YELLOW}========================================${NC}"
echo -e "Total tests:  ${BLUE}$TOTAL_TESTS${NC}"
echo -e "Passed:       ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed:       ${RED}$FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo ""
    echo -e "${GREEN}🎉 ALL TESTS PASSED! 🎉${NC}"
    exit 0
else
    echo ""
    echo -e "${RED}❌ SOME TESTS FAILED${NC}"
    exit 1
fi
