#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

typedef struct Symbol {
    char* name;
    Type type;
    int scope_level;
    struct Symbol* next;
} Symbol;

Symbol* symbol_table = NULL;

int is_declared(const char* name) {
    Symbol* curr = symbol_table;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return 1;
        curr = curr->next;
    }
    return 0;
}

void declare(const char* name, Type type, int scope) {
    Symbol* sym = malloc(sizeof(Symbol));
    sym->name = strdup(name);
    sym->type = type;
    sym->scope_level = scope;
    sym->next = symbol_table;
    symbol_table = sym;
}

void semantic_error(const char* msg, const char* name) {
    fprintf(stderr, "Semantic error: %s '%s'\n", msg, name);
    exit(1);
}

void check_node(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_STMT_LIST:
            for (int i = 0; i < node->stmt_list.count; i++) {
                check_node(node->stmt_list.stmts[i]);
            }
            break;

        case NODE_DECL:
	  if (is_declared(node->decl.var_name)) {
	    semantic_error("Variable redeclared", node->decl.var_name);
	  }
	  declare(node->decl.var_name, node->decl.declared_type, 0);
	  if (node->decl.init_value) {
	    Type init_type = get_type(node->decl.init_value);
	    if (init_type != node->decl.declared_type) {
	      semantic_error("Type mismatch in initialization", node->decl.var_name);
	    }
	  }
	  break;


        case NODE_ASSIGN:
            if (!is_declared(node->assign.var_name)) {
                semantic_error("Assignment to undeclared variable", node->assign.var_name);
            }
            get_type(node->assign.expr);
            break;

        case NODE_PRINT:
            check_node(node->print_expr);
            break;

        case NODE_BINOP:
            check_node(node->binop.left);
            check_node(node->binop.right);
            break;

        case NODE_IF:
            check_node(node->if_stmt.condition);
            check_node(node->if_stmt.if_body);
            if (node->if_stmt.else_body)
                check_node(node->if_stmt.else_body);
            break;

        case NODE_VAR:
            if (!is_declared(node->var_name)) {
                semantic_error("Use of undeclared variable", node->var_name);
            }
            break;

        case NODE_INT:
            break; // always valid

        default:
            fprintf(stderr, "Unknown node type %d\n", node->type);
            break;
    }
}

Type get_type(ASTNode* node) {
    if (!node) return TYPE_ERROR;

    switch (node->type) {
        case NODE_INT:
            return TYPE_INT;
	    
    case NODE_BOOL:
      return TYPE_BOOL;

        case NODE_VAR: {
    if (!is_declared(node->var_name)) {
        semantic_error("Use of undeclared variable", node->var_name);
    }
    Symbol* curr = symbol_table;
    while (curr) {
        if (strcmp(curr->name, node->var_name) == 0) {
            return curr->type;
        }
        curr = curr->next;
    }
    return TYPE_ERROR; // shouldn't happen
}
  

        case NODE_BINOP: {
            Type left = get_type(node->binop.left);
            Type right = get_type(node->binop.right);
            if (left != TYPE_INT || right != TYPE_INT) {
                fprintf(stderr, "Type error: binary operator applied to non-int\n");
                exit(1);
            }
            return TYPE_INT;
        }

       case NODE_ASSIGN: {
    Type rhs = get_type(node->assign.expr);

    Symbol* curr = symbol_table;
    while (curr) {
        if (strcmp(curr->name, node->assign.var_name) == 0) {
            if (curr->type != rhs) {
                semantic_error("Type mismatch in assignment to variable", curr->name);
            }
            return curr->type;
        }
        curr = curr->next;
    }

    semantic_error("Assignment to undeclared variable", node->assign.var_name);
    return TYPE_ERROR;
}

        case NODE_DECL:
            return node->decl.declared_type;

        case NODE_PRINT:
            get_type(node->print_expr);
            return TYPE_INT;

        case NODE_IF:
            get_type(node->if_stmt.condition);
            get_type(node->if_stmt.if_body);
            if (node->if_stmt.else_body)
                get_type(node->if_stmt.else_body);
            return TYPE_INT;

        default:
            fprintf(stderr, "Unknown expression type %d in type check\n", node->type);
            return TYPE_ERROR;
    }
}

void semantic_check(ASTNode* root) {
    check_node(root);
}

void print_symbol_table() {
    printf("Symbol Table:\n");
    printf("%-10s | %-5s | %-5s\n", "Name", "Type", "Scope");
    printf("-----------------------------\n");
    Symbol* curr = symbol_table;
    while (curr) {
        const char* type_str = (curr->type == TYPE_INT) ? "int" :
                       (curr->type == TYPE_BOOL) ? "bool" : "unknown";

        printf("%-10s | %-5s | %-5s\n", curr->name, type_str, "global"); // only one scope for now
        curr = curr->next;
    }
}


