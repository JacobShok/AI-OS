/* Lexer wrapper - maps shell_ prefixed names to yy_ prefixed flex functions */

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#include "Bison.h"

/* Map shell_ prefixed names to yy_ prefixed ones */
#define shell__scan_string yy_scan_string
#define shell__delete_buffer yy_delete_buffer
#define shell_lex_destroy yylex_destroy
#define shell_get_text yyget_text

/* Flex reentrant scanner functions */
extern int yylex_init(yyscan_t* scanner);
extern YY_BUFFER_STATE yy_scan_string(const char *str, yyscan_t scanner);
extern void yy_delete_buffer(YY_BUFFER_STATE buf, yyscan_t scanner);
extern void yylex_destroy(yyscan_t scanner);
extern char* yyget_text(yyscan_t scanner);
extern void yyset_in(FILE* in, yyscan_t scanner);

/* Initialize lexer function */
static inline yyscan_t shell__initialize_lexer(FILE* inp) {
    yyscan_t scanner;
    yylex_init(&scanner);
    if (inp) {
        yyset_in(inp, scanner);
    }
    return scanner;
}

#endif /* LEXER_H */
