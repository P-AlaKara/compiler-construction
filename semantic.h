#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

void semantic_check(ASTNode* root);
Type get_type(ASTNode* node);
void check_node(ASTNode* node);
void print_symbol_table();

#endif
