// ast.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static void* checked_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return ptr;
}

ASTNode* make_int_node(int value) {
    ASTNode* node = checked_malloc(sizeof(ASTNode));
    node->type = NODE_INT;
    node->int_value = value;
    return node;
}
ASTNode* make_bool_node(int value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BOOL;
    node->int_value = value; // same field used for ints
    return node;
}


ASTNode* make_var_node(char* name) {
    ASTNode* node = checked_malloc(sizeof(ASTNode));
    node->type = NODE_VAR;
    node->var_name = strdup(name);
    return node;
}

ASTNode* make_binop_node(char* op, ASTNode* left, ASTNode* right) {
    ASTNode* node = checked_malloc(sizeof(ASTNode));
    node->type = NODE_BINOP;
    node->binop.op = strdup(op);
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode* make_assign_node(char* name, ASTNode* expr) {
    ASTNode* node = checked_malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGN;
    node->assign.var_name = strdup(name);
    node->assign.expr = expr;
    return node;
}

ASTNode* make_declaration_node(char* name, ASTNode* init, Type declared_type) {
    ASTNode* node = checked_malloc(sizeof(ASTNode));
    node->type = NODE_DECL;
    node->decl.var_name = strdup(name);
    node->decl.init_value = init;
    node->decl.declared_type = declared_type;
    return node;
}

ASTNode* make_print_node(ASTNode* expr) {
    ASTNode* node = checked_malloc(sizeof(ASTNode));
    node->type = NODE_PRINT;
    node->print_expr = expr;
    return node;
}

ASTNode* make_if_node(ASTNode* condition, ASTNode* if_body, ASTNode* else_body) {
    ASTNode* node = checked_malloc(sizeof(ASTNode));
    node->type = NODE_IF;
    node->if_stmt.condition = condition;
    node->if_stmt.if_body = if_body;
    node->if_stmt.else_body = else_body;
    return node;
}

ASTNode* make_stmt_list_node() {
    ASTNode* node = checked_malloc(sizeof(ASTNode));
    node->type = NODE_STMT_LIST;
    node->stmt_list.count = 0;
    node->stmt_list.capacity = 4;
    node->stmt_list.stmts = checked_malloc(sizeof(ASTNode*) * node->stmt_list.capacity);
    return node;
}

void add_statement(ASTNode* list, ASTNode* stmt) {
    if (list->type != NODE_STMT_LIST) {
        fprintf(stderr, "Not a statement list\n");
        exit(1);
    }
    if (list->stmt_list.count == list->stmt_list.capacity) {
        list->stmt_list.capacity *= 2;
        list->stmt_list.stmts = realloc(list->stmt_list.stmts, sizeof(ASTNode*) * list->stmt_list.capacity);
    }
    list->stmt_list.stmts[list->stmt_list.count++] = stmt;
}
void print_ast(ASTNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; ++i) printf("  ");

    switch (node->type) {
        case NODE_INT:
            printf("INT: %d\n", node->int_value);
            break;
    case NODE_BOOL:
      printf("BOOL: %d\n", node->int_value);
        case NODE_VAR:
            printf("VAR: %s\n", node->var_name);
            break;
        case NODE_BINOP:
            printf("BINOP: %s\n", node->binop.op);
            print_ast(node->binop.left, indent + 1);
            print_ast(node->binop.right, indent + 1);
            break;
        case NODE_ASSIGN:
            printf("ASSIGN: %s\n", node->assign.var_name);
            print_ast(node->assign.expr, indent + 1);
            break;
        case NODE_DECL:
            printf("DECL: %s\n", node->decl.var_name);
            if (node->decl.init_value)
                print_ast(node->decl.init_value, indent + 1);
            break;
        case NODE_PRINT:
            printf("PRINT:\n");
            print_ast(node->print_expr, indent + 1);
            break;
        case NODE_IF:
            printf("IF:\n");
            print_ast(node->if_stmt.condition, indent + 1);
            printf("THEN:\n");
            print_ast(node->if_stmt.if_body, indent + 1);
            if (node->if_stmt.else_body) {
                printf("ELSE:\n");
                print_ast(node->if_stmt.else_body, indent + 1);
            }
            break;
        case NODE_STMT_LIST:
            printf("STMT_LIST:\n");
            for (int i = 0; i < node->stmt_list.count; i++) {
                print_ast(node->stmt_list.stmts[i], indent + 1);
            }
            break;
    }
}
