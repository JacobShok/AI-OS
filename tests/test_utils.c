/*
 * Unit tests for utility functions
 * Compile: gcc -o test_utils test_utils.c ../src/utils.c -I../include
 * Run: ./test_utils
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counter */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("Testing %s... ", name); \
        tests_run++; \
    } while(0)

#define PASS() \
    do { \
        printf("PASSED\n"); \
        tests_passed++; \
    } while(0)

#define FAIL(msg) \
    do { \
        printf("FAILED: %s\n", msg); \
    } while(0)

/* Test string utilities */
void test_str_ends_with(void)
{
    TEST("str_ends_with");

    assert(str_ends_with("file.txt", ".txt") == 1);
    assert(str_ends_with("file.txt", ".c") == 0);
    assert(str_ends_with("test", "test") == 1);
    assert(str_ends_with("x", "xy") == 0);
    assert(str_ends_with(NULL, ".txt") == 0);
    assert(str_ends_with("file.txt", NULL) == 0);

    PASS();
}

void test_str_starts_with(void)
{
    TEST("str_starts_with");

    assert(str_starts_with("hello world", "hello") == 1);
    assert(str_starts_with("hello", "world") == 0);
    assert(str_starts_with("test", "test") == 1);
    assert(str_starts_with("x", "xy") == 0);
    assert(str_starts_with(NULL, "hello") == 0);
    assert(str_starts_with("hello", NULL) == 0);

    PASS();
}

void test_trim_whitespace(void)
{
    TEST("trim_whitespace");

    char buf1[] = "  hello  ";
    assert(strcmp(trim_whitespace(buf1), "hello") == 0);

    char buf2[] = "hello";
    assert(strcmp(trim_whitespace(buf2), "hello") == 0);

    char buf3[] = "   ";
    assert(strcmp(trim_whitespace(buf3), "") == 0);

    char buf4[] = "  hello world  ";
    assert(strcmp(trim_whitespace(buf4), "hello world") == 0);

    PASS();
}

/* Test path manipulation */
void test_path_join(void)
{
    TEST("path_join");

    char *p1 = path_join("/usr", "bin");
    assert(strcmp(p1, "/usr/bin") == 0);
    free(p1);

    char *p2 = path_join("/usr/", "bin");
    assert(strcmp(p2, "/usr/bin") == 0);
    free(p2);

    char *p3 = path_join("", "bin");
    assert(strcmp(p3, "bin") == 0);
    free(p3);

    PASS();
}

void test_get_basename(void)
{
    TEST("get_basename");

    char *b1 = get_basename("/path/to/file.txt");
    assert(strcmp(b1, "file.txt") == 0);
    free(b1);

    char *b2 = get_basename("file.txt");
    assert(strcmp(b2, "file.txt") == 0);
    free(b2);

    char *b3 = get_basename("/path/to/");
    assert(strcmp(b3, "to") == 0);
    free(b3);

    PASS();
}

void test_get_dirname(void)
{
    TEST("get_dirname");

    char *d1 = get_dirname("/path/to/file.txt");
    assert(strcmp(d1, "/path/to") == 0);
    free(d1);

    char *d2 = get_dirname("file.txt");
    assert(strcmp(d2, ".") == 0);
    free(d2);

    PASS();
}

/* Test formatting functions */
void test_format_size(void)
{
    TEST("format_size");

    char buf[32];

    format_size(512, buf, sizeof(buf));
    assert(strcmp(buf, "512B") == 0);

    format_size(1536, buf, sizeof(buf));
    assert(strcmp(buf, "1.5K") == 0);

    format_size(1048576, buf, sizeof(buf));
    assert(strcmp(buf, "1.0M") == 0);

    format_size(1536 * 1024, buf, sizeof(buf));
    assert(strcmp(buf, "1.5M") == 0);

    PASS();
}

/* Main test runner */
int main(void)
{
    printf("=== PicoBox Utility Function Tests ===\n\n");

    /* Run all tests */
    test_str_ends_with();
    test_str_starts_with();
    test_trim_whitespace();
    test_path_join();
    test_get_basename();
    test_get_dirname();
    test_format_size();

    /* Print summary */
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    if (tests_passed == tests_run) {
        printf("\nAll tests PASSED! ✓\n");
        return 0;
    } else {
        printf("\nSome tests FAILED! ✗\n");
        return 1;
    }
}
