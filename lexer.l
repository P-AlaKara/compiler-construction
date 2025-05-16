/* lexer.l */

%{
#include "parser.tab.h" // Include the header file Bison will generate
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// Declare yyin - input file pointer, used by Lex/Flex.
extern FILE *yyin;

// **REQUIRED:** Declare yylineno - managed by Lex/Flex when %option yylineno is used
extern int yylineno; // <-- Declare yylineno here

/* Remove 'int current_line = 1;' */
/* Remove '// extern int yylineno;' */

%}

/* Definitions section */

%option noyywrap
%option yylineno   

%% /* Rules section */

"if" { return IF; }
"else" { return ELSE; }
"print" { return PRINT; }
"int" {return INT_KEYWORD; }
"bool"   { return BOOL_KEYWORD; }
"true"   { yylval.ival = 1; return BOOLEAN_LITERAL; }
"false"  { yylval.ival = 0; return BOOLEAN_LITERAL; }

"=="|"!="|"<="|">="|"<"|">" { yylval.sval = strdup(yytext); return COMPARISON_OPERATOR; }
"=" { yylval.sval = strdup(yytext); return ASSIGNMENT_OPERATOR; }

"+" { yylval.sval = strdup(yytext); return PLUS; }
"-" { yylval.sval = strdup(yytext); return MINUS; }
"*" { yylval.sval = strdup(yytext); return TIMES; }
"/" { yylval.sval = strdup(yytext); return DIVIDE; }

"(" { return LEFT_PAREN; }
")" { return RIGHT_PAREN; }
"{" { return LEFT_BRACE; }
"}" { return RIGHT_BRACE; }
";" { return SEMICOLON; }

[0-9]+  { yylval.ival = atoi(yytext); return CONSTANT; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.sval = strdup(yytext); return IDENTIFIER; }

"//".*  { /* Skip comment */ }
[ \t]+  { /* Skip spaces and tabs */ }
\n   { /* Lex automatically updates yylineno because of %option yylineno */ } 

. {fprintf(stderr, "Lexical Error: Unknown character '%s' at line %d\n", yytext, yylineno);
 exit(1);
}

%% 

