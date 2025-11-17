/* Compatibility layer for shell_ prefixed functions */

#include <stdio.h>

/* Define flex types before including Bison.h */
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
#endif

#include "Absyn.h"
#include "Bison.h"

/* Forward declarations from flex */
extern int yylex_init(yyscan_t* scanner);
extern YY_BUFFER_STATE yy_scan_string(const char *str, yyscan_t scanner);
extern void yy_delete_buffer(YY_BUFFER_STATE buf, yyscan_t scanner);
extern int yylex_destroy(yyscan_t scanner);
extern char* yyget_text(yyscan_t scanner);
extern void yyset_in(FILE* in, yyscan_t scanner);

/* Wrapper functions with shell_ prefix */
/* Note: shell__initialize_lexer is defined in Shell.l */

YY_BUFFER_STATE shell__scan_string(const char *str, yyscan_t scanner) {
    return yy_scan_string(str, scanner);
}

void shell__delete_buffer(YY_BUFFER_STATE buf, yyscan_t scanner) {
    yy_delete_buffer(buf, scanner);
}

void shell_lex_destroy(yyscan_t scanner) {
    yylex_destroy(scanner);
}

char* shell_get_text(yyscan_t scanner) {
    return yyget_text(scanner);
}
