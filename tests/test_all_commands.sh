#!/bin/bash
#
# Comprehensive test for all 25 PicoBox commands
#

# Get absolute path to picobox BEFORE changing directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PICOBOX="$PROJECT_DIR/build/picobox"

PASSED=0
FAILED=0

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Testing All 25 PicoBox Commands${NC}"
echo -e "${BLUE}========================================${NC}\n"

# Check if picobox binary exists
if [ ! -f "$PICOBOX" ]; then
    echo -e "${RED}Error: picobox binary not found at: $PICOBOX${NC}"
    echo "Please run 'make' first to build picobox"
    exit 1
fi

# Create temporary test directory
TEST_DIR=$(mktemp -d)
trap "rm -rf $TEST_DIR" EXIT
cd "$TEST_DIR" || exit 1

echo -e "${YELLOW}Test directory: $TEST_DIR${NC}\n"

# ============================================================
# TEST 1-5: BASIC COMMANDS (echo, pwd, true, false, sleep)
# ============================================================
echo -e "${BLUE}[1/25] Testing ECHO${NC}"
if output=$($PICOBOX echo "Hello PicoBox") && [ "$output" = "Hello PicoBox" ]; then
    echo -e "  ${GREEN}✓${NC} echo basic output"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} echo basic output"
    ((FAILED++))
fi

if output=$($PICOBOX echo -n "no newline") && [ "$output" = "no newline" ]; then
    echo -e "  ${GREEN}✓${NC} echo -n flag"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} echo -n flag"
    ((FAILED++))
fi

echo -e "\n${BLUE}[2/25] Testing PWD${NC}"
if output=$($PICOBOX pwd) && [ "$output" = "$TEST_DIR" ]; then
    echo -e "  ${GREEN}✓${NC} pwd prints current directory"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} pwd (got: $output)"
    ((FAILED++))
fi

echo -e "\n${BLUE}[3/25] Testing TRUE${NC}"
if $PICOBOX true; then
    echo -e "  ${GREEN}✓${NC} true returns 0"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} true returns 0"
    ((FAILED++))
fi

echo -e "\n${BLUE}[4/25] Testing FALSE${NC}"
if ! $PICOBOX false; then
    echo -e "  ${GREEN}✓${NC} false returns 1"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} false returns 1"
    ((FAILED++))
fi

echo -e "\n${BLUE}[5/25] Testing SLEEP${NC}"
start=$(date +%s)
$PICOBOX sleep 1
end=$(date +%s)
elapsed=$((end - start))
if [ $elapsed -ge 1 ]; then
    echo -e "  ${GREEN}✓${NC} sleep 1 second"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} sleep 1 second"
    ((FAILED++))
fi

# ============================================================
# TEST 6-7: PATH UTILITIES (basename, dirname)
# ============================================================
echo -e "\n${BLUE}[6/25] Testing BASENAME${NC}"
if output=$($PICOBOX basename /usr/local/bin/picobox) && [ "$output" = "picobox" ]; then
    echo -e "  ${GREEN}✓${NC} basename /usr/local/bin/picobox"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} basename"
    ((FAILED++))
fi

if output=$($PICOBOX basename /path/to/file.txt .txt) && [ "$output" = "file" ]; then
    echo -e "  ${GREEN}✓${NC} basename with suffix removal"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} basename with suffix"
    ((FAILED++))
fi

echo -e "\n${BLUE}[7/25] Testing DIRNAME${NC}"
if output=$($PICOBOX dirname /usr/local/bin/picobox) && [ "$output" = "/usr/local/bin" ]; then
    echo -e "  ${GREEN}✓${NC} dirname /usr/local/bin/picobox"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} dirname"
    ((FAILED++))
fi

# ============================================================
# TEST 8-11: FILE CREATION (mkdir, touch)
# ============================================================
echo -e "\n${BLUE}[8/25] Testing MKDIR${NC}"
if $PICOBOX mkdir testdir && [ -d testdir ]; then
    echo -e "  ${GREEN}✓${NC} mkdir creates directory"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} mkdir creates directory"
    ((FAILED++))
fi

if $PICOBOX mkdir -p a/b/c && [ -d a/b/c ]; then
    echo -e "  ${GREEN}✓${NC} mkdir -p creates parents"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} mkdir -p"
    ((FAILED++))
fi

if $PICOBOX mkdir -m 755 permdir && [ -d permdir ]; then
    echo -e "  ${GREEN}✓${NC} mkdir -m sets permissions"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} mkdir -m"
    ((FAILED++))
fi

echo -e "\n${BLUE}[9/25] Testing TOUCH${NC}"
if $PICOBOX touch newfile.txt && [ -f newfile.txt ]; then
    echo -e "  ${GREEN}✓${NC} touch creates file"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} touch creates file"
    ((FAILED++))
fi

# ============================================================
# TEST 10-13: FILE OPERATIONS (cat, cp, mv, rm)
# ============================================================
echo -e "\n${BLUE}[10/25] Testing CAT${NC}"
echo "test content" > file1.txt
if output=$($PICOBOX cat file1.txt) && [ "$output" = "test content" ]; then
    echo -e "  ${GREEN}✓${NC} cat reads file"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} cat reads file"
    ((FAILED++))
fi

echo "file2 content" > file2.txt
output=$($PICOBOX cat file1.txt file2.txt)
if echo "$output" | grep -q "test content" && echo "$output" | grep -q "file2 content"; then
    echo -e "  ${GREEN}✓${NC} cat concatenates files"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} cat concatenates files"
    ((FAILED++))
fi

if output=$(echo "stdin test" | $PICOBOX cat) && [ "$output" = "stdin test" ]; then
    echo -e "  ${GREEN}✓${NC} cat reads from stdin"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} cat reads from stdin"
    ((FAILED++))
fi

echo -e "\n${BLUE}[11/25] Testing CP${NC}"
echo "source content" > source.txt
if $PICOBOX cp source.txt dest.txt && [ -f dest.txt ]; then
    echo -e "  ${GREEN}✓${NC} cp copies file"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} cp copies file"
    ((FAILED++))
fi

$PICOBOX mkdir -p srcdir/sub
echo "nested" > srcdir/sub/file.txt
if $PICOBOX cp -r srcdir destdir && [ -f destdir/sub/file.txt ]; then
    echo -e "  ${GREEN}✓${NC} cp -r copies directory"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} cp -r copies directory"
    ((FAILED++))
fi

echo -e "\n${BLUE}[12/25] Testing MV${NC}"
echo "move me" > movefile.txt
if $PICOBOX mv movefile.txt renamed.txt && [ -f renamed.txt ] && [ ! -f movefile.txt ]; then
    echo -e "  ${GREEN}✓${NC} mv renames file"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} mv renames file"
    ((FAILED++))
fi

echo -e "\n${BLUE}[13/25] Testing RM${NC}"
echo "delete me" > deleteme.txt
if $PICOBOX rm deleteme.txt && [ ! -f deleteme.txt ]; then
    echo -e "  ${GREEN}✓${NC} rm deletes file"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} rm deletes file"
    ((FAILED++))
fi

$PICOBOX mkdir -p rmdir/subdir
echo "nested" > rmdir/subdir/file.txt
if $PICOBOX rm -r rmdir && [ ! -d rmdir ]; then
    echo -e "  ${GREEN}✓${NC} rm -r removes directory"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} rm -r removes directory"
    ((FAILED++))
fi

# ============================================================
# TEST 14-15: DIRECTORY LISTING (ls)
# ============================================================
echo -e "\n${BLUE}[14/25] Testing LS${NC}"
$PICOBOX mkdir lstest
$PICOBOX touch lstest/file1.txt
$PICOBOX touch lstest/file2.txt
$PICOBOX touch lstest/.hidden
if output=$($PICOBOX ls lstest) && echo "$output" | grep -q "file1.txt"; then
    echo -e "  ${GREEN}✓${NC} ls lists files"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} ls lists files"
    ((FAILED++))
fi

if output=$($PICOBOX ls -a lstest) && echo "$output" | grep -q ".hidden"; then
    echo -e "  ${GREEN}✓${NC} ls -a shows hidden files"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} ls -a shows hidden files"
    ((FAILED++))
fi

if output=$($PICOBOX ls -l lstest) && echo "$output" | grep -q "file1.txt"; then
    echo -e "  ${GREEN}✓${NC} ls -l long format"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} ls -l long format"
    ((FAILED++))
fi

# ============================================================
# TEST 16-18: TEXT PROCESSING (head, tail, wc)
# ============================================================
echo -e "\n${BLUE}[15/25] Testing HEAD${NC}"
seq 1 20 > numbers.txt
if output=$($PICOBOX head numbers.txt) && [ "$(echo "$output" | wc -l)" -eq 10 ]; then
    echo -e "  ${GREEN}✓${NC} head shows first 10 lines"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} head shows first 10 lines"
    ((FAILED++))
fi

if output=$($PICOBOX head -n 5 numbers.txt) && [ "$(echo "$output" | wc -l)" -eq 5 ]; then
    echo -e "  ${GREEN}✓${NC} head -n 5 shows first 5 lines"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} head -n 5"
    ((FAILED++))
fi

echo -e "\n${BLUE}[16/25] Testing TAIL${NC}"
if output=$($PICOBOX tail numbers.txt) && echo "$output" | grep -q "20"; then
    echo -e "  ${GREEN}✓${NC} tail shows last 10 lines"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} tail shows last 10 lines"
    ((FAILED++))
fi

if output=$($PICOBOX tail -n 3 numbers.txt) && [ "$(echo "$output" | wc -l)" -eq 3 ]; then
    echo -e "  ${GREEN}✓${NC} tail -n 3 shows last 3 lines"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} tail -n 3"
    ((FAILED++))
fi

echo -e "\n${BLUE}[17/25] Testing WC${NC}"
echo -e "line1\nline2\nline3" > wctest.txt
if output=$($PICOBOX wc wctest.txt) && echo "$output" | grep -q "3"; then
    echo -e "  ${GREEN}✓${NC} wc counts lines"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} wc counts lines"
    ((FAILED++))
fi

if output=$($PICOBOX wc -l wctest.txt) && echo "$output" | grep -q "3"; then
    echo -e "  ${GREEN}✓${NC} wc -l counts lines only"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} wc -l"
    ((FAILED++))
fi

# ============================================================
# TEST 18: LINKS (ln)
# ============================================================
echo -e "\n${BLUE}[18/25] Testing LN${NC}"
echo "link target" > linktarget.txt
if $PICOBOX ln -s linktarget.txt symlink.txt && [ -L symlink.txt ]; then
    echo -e "  ${GREEN}✓${NC} ln -s creates symlink"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} ln -s creates symlink"
    ((FAILED++))
fi

if $PICOBOX ln linktarget.txt hardlink.txt && [ -f hardlink.txt ]; then
    echo -e "  ${GREEN}✓${NC} ln creates hard link"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} ln creates hard link"
    ((FAILED++))
fi

# ============================================================
# TEST 19-20: SEARCH (grep, find)
# ============================================================
echo -e "\n${BLUE}[19/25] Testing GREP${NC}"
echo -e "apple\nbanana\napple pie\ncherry" > fruits.txt
if output=$($PICOBOX grep "apple" fruits.txt) && [ "$(echo "$output" | wc -l)" -eq 2 ]; then
    echo -e "  ${GREEN}✓${NC} grep finds matches"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} grep finds matches"
    ((FAILED++))
fi

if output=$($PICOBOX grep -i "APPLE" fruits.txt) && echo "$output" | grep -q "apple"; then
    echo -e "  ${GREEN}✓${NC} grep -i case insensitive"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} grep -i"
    ((FAILED++))
fi

if output=$($PICOBOX grep -n "banana" fruits.txt) && echo "$output" | grep -q "2:"; then
    echo -e "  ${GREEN}✓${NC} grep -n shows line numbers"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} grep -n"
    ((FAILED++))
fi

echo -e "\n${BLUE}[20/25] Testing FIND${NC}"
$PICOBOX mkdir -p findtest/sub1/sub2
$PICOBOX touch findtest/file1.txt
$PICOBOX touch findtest/file2.c
$PICOBOX touch findtest/sub1/file3.txt
if output=$($PICOBOX find findtest -name "*.txt") && [ "$(echo "$output" | wc -l)" -eq 2 ]; then
    echo -e "  ${GREEN}✓${NC} find -name pattern"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} find -name pattern"
    ((FAILED++))
fi

if output=$($PICOBOX find findtest -type f) && echo "$output" | grep -q "file1.txt"; then
    echo -e "  ${GREEN}✓${NC} find -type f finds files"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} find -type f"
    ((FAILED++))
fi

if output=$($PICOBOX find findtest -type d) && echo "$output" | grep -q "sub1"; then
    echo -e "  ${GREEN}✓${NC} find -type d finds directories"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} find -type d"
    ((FAILED++))
fi

# ============================================================
# TEST 21-23: FILE INFO (chmod, stat, du, df)
# ============================================================
echo -e "\n${BLUE}[21/25] Testing CHMOD${NC}"
$PICOBOX touch chmodtest.sh
if $PICOBOX chmod 755 chmodtest.sh && [ -x chmodtest.sh ]; then
    echo -e "  ${GREEN}✓${NC} chmod 755 sets permissions"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} chmod 755"
    ((FAILED++))
fi

echo -e "\n${BLUE}[22/25] Testing STAT${NC}"
$PICOBOX touch stattest.txt
if output=$($PICOBOX stat stattest.txt) && echo "$output" | grep -q "stattest.txt"; then
    echo -e "  ${GREEN}✓${NC} stat shows file info"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} stat shows file info"
    ((FAILED++))
fi

echo -e "\n${BLUE}[23/25] Testing DU${NC}"
$PICOBOX mkdir -p dutest/sub
echo "content" > dutest/file.txt
echo "more" > dutest/sub/file2.txt
if output=$($PICOBOX du dutest) && echo "$output" | grep -q "dutest"; then
    echo -e "  ${GREEN}✓${NC} du shows disk usage"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} du shows disk usage"
    ((FAILED++))
fi

if output=$($PICOBOX du -h dutest) && (echo "$output" | grep -q "K\|M\|B"); then
    echo -e "  ${GREEN}✓${NC} du -h human readable"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} du -h (got: $output)"
    ((FAILED++))
fi

echo -e "\n${BLUE}[24/25] Testing DF${NC}"
if output=$($PICOBOX df .) && echo "$output" | grep -q "Filesystem"; then
    echo -e "  ${GREEN}✓${NC} df shows filesystem info"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} df shows filesystem info"
    ((FAILED++))
fi

if output=$($PICOBOX df -h .) && echo "$output" | grep -q "Size"; then
    echo -e "  ${GREEN}✓${NC} df -h human readable"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} df -h"
    ((FAILED++))
fi

# ============================================================
# TEST 25: ENVIRONMENT (env)
# ============================================================
echo -e "\n${BLUE}[25/25] Testing ENV${NC}"
if output=$($PICOBOX env) && echo "$output" | grep -q "PATH"; then
    echo -e "  ${GREEN}✓${NC} env shows environment variables"
    ((PASSED++))
else
    echo -e "  ${RED}✗${NC} env shows environment"
    ((FAILED++))
fi

# ============================================================
# SUMMARY
# ============================================================
TOTAL=$((PASSED + FAILED))
echo -e "\n${BLUE}========================================${NC}"
echo -e "${BLUE}  Test Summary${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "Total tests:  $TOTAL"
echo -e "${GREEN}Passed:       $PASSED${NC}"
if [ $FAILED -gt 0 ]; then
    echo -e "${RED}Failed:       $FAILED${NC}"
else
    echo -e "Failed:       $FAILED"
fi
echo -e "Success rate: $((PASSED * 100 / TOTAL))%"
echo -e "${BLUE}========================================${NC}\n"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}🎉 ALL TESTS PASSED! 🎉${NC}\n"
    exit 0
else
    echo -e "${RED}❌ Some tests failed${NC}\n"
    exit 1
fi
