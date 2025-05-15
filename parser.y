/* parser.y */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "semantic.h"

ASTNode* root = NULL;
int yylex(void);
void yyerror(const char *s);
extern FILE *yyin;
extern int yylineno;

%}

%union {
    char* sval;
    int ival;
    struct ASTNode* node;
}

%token <sval> IDENTIFIER COMPARISON_OPERATOR ASSIGNMENT_OPERATOR
%token <sval> PLUS MINUS TIMES DIVIDE
%token IF ELSE PRINT INT_KEYWORD
%token LEFT_PAREN RIGHT_PAREN LEFT_BRACE RIGHT_BRACE SEMICOLON
%token <ival> CONSTANT

%type <node> program statement_list statement declaration assignment comparison expression

%start program

%left PLUS MINUS
%left TIMES DIVIDE

%%

program:
    statement_list                     { extern ASTNode* root;
        root = $1; }
;

statement_list:
      /* empty */                     { $$ = make_stmt_list_node(); }
    | statement_list statement        { add_statement($1, $2); $$ = $1; }
;

statement:
      declaration SEMICOLON           { $$ = $1; }
    | assignment SEMICOLON            { $$ = $1; }
    | PRINT expression SEMICOLON      { $$ = make_print_node($2); }
    | IF LEFT_PAREN comparison RIGHT_PAREN LEFT_BRACE statement_list RIGHT_BRACE ELSE LEFT_BRACE statement_list RIGHT_BRACE
                                      { $$ = make_if_node($3, $6, $10); }
    | IF LEFT_PAREN comparison RIGHT_PAREN LEFT_BRACE statement_list RIGHT_BRACE
                                      { $$ = make_if_node($3, $6, NULL); }
;

declaration:
      INT_KEYWORD IDENTIFIER             { $$ = make_declaration_node($2, NULL); }
    | INT_KEYWORD IDENTIFIER ASSIGNMENT_OPERATOR expression
                                      { $$ = make_declaration_node($2, $4); }
;

assignment:
    IDENTIFIER ASSIGNMENT_OPERATOR expression
                                      { $$ = make_assign_node($1, $3); }
;

comparison:
    expression COMPARISON_OPERATOR expression
                                      { $$ = make_binop_node($2, $1, $3); }
;

expression:
      CONSTANT                        { $$ = make_int_node($1); }
    | IDENTIFIER                      { $$ = make_var_node($1); }
    | expression PLUS expression      { $$ = make_binop_node("+", $1, $3); }
    | expression MINUS expression     { $$ = make_binop_node("-", $1, $3); }
    | expression TIMES expression     { $$ = make_binop_node("*", $1, $3); }
    | expression DIVIDE expression    { $$ = make_binop_node("/", $1, $3); }
    | LEFT_PAREN expression RIGHT_PAREN
                                      { $$ = $2; }
;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error: %s at line %d\n", s, yylineno);
}

int main(int argc, char** argv) {
    if (argc > 1) {
        FILE* file = fopen(argv[1], "r");
        if (!file) {
            perror("fopen");
            return 1;
        }
        yyin = file;
    }

    printf("Parsing...\n");
    yyparse();
    printf("Parsing finished.\n");

    // if you stored the root AST node from the parser, you'd print it here
    print_ast(root, 0);
    semantic_check(root);

    printf("semantic analysis passed");

    return 0;
}
