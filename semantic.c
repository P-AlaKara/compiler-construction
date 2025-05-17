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
    printf("DEBUG: Declaring symbol '%s' with type %d\n", name, type);
    Symbol* sym = malloc(sizeof(Symbol));
    if (!sym) {
        fprintf(stderr, "Memory allocation failed for symbol\n");
        exit(1);
    }
    sym->name = strdup(name);
    sym->type = type;
    sym->scope_level = scope;
    sym->next = symbol_table;
    symbol_table = sym;
    printf("DEBUG: Symbol declared successfully\n");
}

void semantic_error(const char* msg, const char* name) {
    fprintf(stderr, "Semantic error: %s '%s'\n", msg, name);
    exit(1);
}

Type get_type(ASTNode* node) {
    if (!node) {
        printf("DEBUG: get_type called with NULL node\n");
        return TYPE_ERROR;
    }

    printf("DEBUG: get_type processing node of type %d\n", node->type);

    switch (node->type) {
        case NODE_INT:
            printf("DEBUG: get_type - found INT node with value %d\n", node->int_value);
            return TYPE_INT;
            
        case NODE_BOOL:
            printf("DEBUG: get_type - found BOOL node with value %d\n", node->int_value);
            return TYPE_BOOL;

        case NODE_VAR: {
            printf("DEBUG: get_type - checking variable '%s'\n", node->var_name);
            if (!is_declared(node->var_name)) {
                semantic_error("Use of undeclared variable", node->var_name);
            }
            Symbol* curr = symbol_table;
            while (curr) {
                if (strcmp(curr->name, node->var_name) == 0) {
                    printf("DEBUG: get_type - variable '%s' has type %d\n", node->var_name, curr->type);
                    return curr->type;
                }
                curr = curr->next;
            }
            return TYPE_ERROR; // shouldn't happen
        }

        case NODE_BINOP: {
            printf("DEBUG: get_type - processing binary operation '%s'\n", node->binop.op);
            Type left = get_type(node->binop.left);
            Type right = get_type(node->binop.right);
            if (left != TYPE_INT || right != TYPE_INT) {
                fprintf(stderr, "Type error: binary operator applied to non-int\n");
                exit(1);
            }
            return TYPE_INT;
        }

        case NODE_ASSIGN: {
            printf("DEBUG: get_type - processing assignment to '%s'\n", node->assign.var_name);
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
            printf("DEBUG: get_type - processing declaration of '%s' with type %d\n", 
                  node->decl.var_name, node->decl.declared_type);
            return node->decl.declared_type;

        case NODE_PRINT:
            printf("DEBUG: get_type - processing print statement\n");
            get_type(node->print_expr);
            return TYPE_INT;

        case NODE_IF:
            printf("DEBUG: get_type - processing if statement\n");
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

void check_node(ASTNode* node) {
    if (!node) {
        printf("DEBUG: check_node called with NULL node\n");
        return;
    }

    printf("DEBUG: check_node processing node of type %d\n", node->type);

    switch (node->type) {
        case NODE_STMT_LIST:
            printf("DEBUG: check_node - processing statement list with %d statements\n", 
                   node->stmt_list.count);
            for (int i = 0; i < node->stmt_list.count; i++) {
                check_node(node->stmt_list.stmts[i]);
            }
            break;

        case NODE_DECL:
            printf("DEBUG: check_node - processing declaration of '%s'\n", node->decl.var_name);
            if (is_declared(node->decl.var_name)) {
                semantic_error("Variable redeclared", node->decl.var_name);
            }
            declare(node->decl.var_name, node->decl.declared_type, 0);
            if (node->decl.init_value) {
                printf("DEBUG: About to get type of initializer for %s, node type: %d\n", 
                       node->decl.var_name, node->decl.init_value->type);
                printf("DEBUG: Initializer address: %p\n", (void*)node->decl.init_value);
                Type init_type = get_type(node->decl.init_value);
                printf("DEBUG: Got type %d for initializer\n", init_type);
                if (init_type != node->decl.declared_type) {
                    semantic_error("Type mismatch in initialization", node->decl.var_name);
                }
            }
            break;

        case NODE_ASSIGN:
            printf("DEBUG: check_node - processing assignment to '%s'\n", node->assign.var_name);
            if (!is_declared(node->assign.var_name)) {
                semantic_error("Assignment to undeclared variable", node->assign.var_name);
            }
            get_type(node->assign.expr);
            break;

        case NODE_PRINT:
            printf("DEBUG: check_node - processing print statement\n");
            check_node(node->print_expr);
            break;

        case NODE_BINOP:
            printf("DEBUG: check_node - processing binary operation\n");
            check_node(node->binop.left);
            check_node(node->binop.right);
            break;

        case NODE_IF:
            printf("DEBUG: check_node - processing if statement\n");
            check_node(node->if_stmt.condition);
            check_node(node->if_stmt.if_body);
            if (node->if_stmt.else_body)
                check_node(node->if_stmt.else_body);
            break;

        case NODE_VAR:
            printf("DEBUG: check_node - processing variable '%s'\n", node->var_name);
            if (!is_declared(node->var_name)) {
                semantic_error("Use of undeclared variable", node->var_name);
            }
            break;

        case NODE_INT:
            printf("DEBUG: check_node - processing integer value: %d\n", node->int_value);
            break;
            
        case NODE_BOOL:
            printf("DEBUG: check_node - processing boolean value: %d\n", node->int_value);
            break;

        default:
            fprintf(stderr, "Unknown node type %d\n", node->type);
            break;
    }
}

void semantic_check(ASTNode* root) {
    printf("Starting semantic check...\n");
    if (!root) {
        printf("ERROR: semantic_check received NULL root\n");
        return;
    }
    printf("Root node type: %d\n", root->type);
    
    // If it's a statement list, let's print debug info about it
    if (root->type == NODE_STMT_LIST) {
        printf("Statement list has %d statements\n", root->stmt_list.count);
        for (int i = 0; i < root->stmt_list.count; i++) {
            if (root->stmt_list.stmts[i]) {
                printf("Statement %d has type %d\n", i, root->stmt_list.stmts[i]->type);
            } else {
                printf("Statement %d is NULL\n", i);
            }
        }
    }
    
    check_node(root);
    printf("Semantic check completed.\n");
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
