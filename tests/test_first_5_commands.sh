#!/bin/bash
#
# Test the first 5 implemented commands
#

PICOBOX="./build/picobox"
PASSED=0
FAILED=0

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

echo "=== Testing First 5 Commands ==="
echo

# Create temporary test directory
TEST_DIR=$(mktemp -d)
trap "rm -rf $TEST_DIR" EXIT

cd "$TEST_DIR" || exit 1

echo -e "${BLUE}Test Directory: $TEST_DIR${NC}\n"

# ===== TEST ECHO =====
echo -e "${BLUE}===== Testing ECHO =====${NC}"

# Test 1: Basic echo
if output=$($PICOBOX echo hello world) && [ "$output" = "hello world" ]; then
    echo -e "${GREEN}✓${NC} echo hello world"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} echo hello world (got: '$output')"
    ((FAILED++))
fi

# Test 2: Echo with -n
if output=$($PICOBOX echo -n "no newline") && [ "$output" = "no newline" ]; then
    echo -e "${GREEN}✓${NC} echo -n"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} echo -n"
    ((FAILED++))
fi

# Test 3: Echo empty
if output=$($PICOBOX echo) && [ "$output" = "" ]; then
    echo -e "${GREEN}✓${NC} echo (empty)"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} echo (empty)"
    ((FAILED++))
fi

# ===== TEST PWD =====
echo -e "\n${BLUE}===== Testing PWD =====${NC}"

# Test 4: pwd prints current directory
if output=$($PICOBOX pwd) && [ "$output" = "$TEST_DIR" ]; then
    echo -e "${GREEN}✓${NC} pwd"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} pwd (expected: $TEST_DIR, got: $output)"
    ((FAILED++))
fi

# ===== TEST MKDIR =====
echo -e "\n${BLUE}===== Testing MKDIR =====${NC}"

# Test 5: Create single directory
if $PICOBOX mkdir testdir && [ -d testdir ]; then
    echo -e "${GREEN}✓${NC} mkdir testdir"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} mkdir testdir"
    ((FAILED++))
fi

# Test 6: Create nested directories with -p
if $PICOBOX mkdir -p a/b/c && [ -d a/b/c ]; then
    echo -e "${GREEN}✓${NC} mkdir -p a/b/c"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} mkdir -p a/b/c"
    ((FAILED++))
fi

# Test 7: mkdir with permissions
if $PICOBOX mkdir -m 755 permdir && [ -d permdir ]; then
    echo -e "${GREEN}✓${NC} mkdir -m 755 permdir"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} mkdir -m 755 permdir"
    ((FAILED++))
fi

# ===== TEST TOUCH =====
echo -e "\n${BLUE}===== Testing TOUCH =====${NC}"

# Test 8: Create new file
if $PICOBOX touch newfile.txt && [ -f newfile.txt ]; then
    echo -e "${GREEN}✓${NC} touch newfile.txt"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} touch newfile.txt"
    ((FAILED++))
fi

# Test 9: Update existing file timestamp
sleep 1
BEFORE=$(stat -f %m newfile.txt 2>/dev/null || stat -c %Y newfile.txt 2>/dev/null)
sleep 1
$PICOBOX touch newfile.txt
AFTER=$(stat -f %m newfile.txt 2>/dev/null || stat -c %Y newfile.txt 2>/dev/null)
if [ "$AFTER" -gt "$BEFORE" ]; then
    echo -e "${GREEN}✓${NC} touch updates timestamp"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} touch updates timestamp"
    ((FAILED++))
fi

# Test 10: touch -c doesn't create
rm -f nonexistent.txt
$PICOBOX touch -c nonexistent.txt
if [ ! -f nonexistent.txt ]; then
    echo -e "${GREEN}✓${NC} touch -c doesn't create file"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} touch -c doesn't create file"
    ((FAILED++))
fi

# ===== TEST CAT =====
echo -e "\n${BLUE}===== Testing CAT =====${NC}"

# Test 11: Cat single file
echo "Hello, World!" > test.txt
if output=$($PICOBOX cat test.txt) && [ "$output" = "Hello, World!" ]; then
    echo -e "${GREEN}✓${NC} cat test.txt"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} cat test.txt (got: '$output')"
    ((FAILED++))
fi

# Test 12: Cat multiple files
echo "File 1" > file1.txt
echo "File 2" > file2.txt
if output=$($PICOBOX cat file1.txt file2.txt) && [ "$output" = "File 1
File 2" ]; then
    echo -e "${GREEN}✓${NC} cat file1.txt file2.txt"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} cat multiple files"
    ((FAILED++))
fi

# Test 13: Cat from stdin
if output=$(echo "stdin test" | $PICOBOX cat) && [ "$output" = "stdin test" ]; then
    echo -e "${GREEN}✓${NC} cat from stdin"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} cat from stdin"
    ((FAILED++))
fi

# Test 14: Cat with line numbers
echo -e "line1\nline2\nline3" > lines.txt
if output=$($PICOBOX cat -n lines.txt | head -1 | grep -q "1  line1"); then
    echo -e "${GREEN}✓${NC} cat -n (line numbers)"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} cat -n (line numbers)"
    ((FAILED++))
fi

# Test 15: Cat nonexistent file (should fail gracefully)
if ! $PICOBOX cat nonexistent.txt 2>/dev/null; then
    echo -e "${GREEN}✓${NC} cat handles missing file"
    ((PASSED++))
else
    echo -e "${RED}✗${NC} cat handles missing file"
    ((FAILED++))
fi

# ===== SUMMARY =====
echo
echo "======================================"
echo "Test Summary:"
echo "  Passed: $PASSED"
echo "  Failed: $FAILED"
echo "  Total:  $((PASSED + FAILED))"
echo "======================================"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests PASSED!${NC}"
    exit 0
else
    echo -e "${RED}Some tests FAILED!${NC}"
    exit 1
fi
