// codegen.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

#define MAX_TAC 1000

typedef struct {
    char op[8];
    char arg1[32];
    char arg2[32];
    char result[32];
} TACInstruction;

TACInstruction tac[MAX_TAC];
int tac_index = 0;
int temp_index = 0;

char* new_temp() {
    static char temp[32];
    snprintf(temp, sizeof(temp), "t%d", temp_index++);
    return strdup(temp);
}

char* generate_expr(ASTNode* node);

void generate_stmt(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_STMT_LIST:
            for (int i = 0; i < node->stmt_list.count; i++) {
                generate_stmt(node->stmt_list.stmts[i]);
            }
            break;

        case NODE_DECL:
            if (node->decl.init_value) {
                char* val = generate_expr(node->decl.init_value);
                snprintf(tac[tac_index].op, 8, "=");
                strcpy(tac[tac_index].arg1, val);
                tac[tac_index].arg2[0] = '\0';
                strcpy(tac[tac_index].result, node->decl.var_name);
                tac_index++;
            }
            break;

        case NODE_ASSIGN: {
            char* val = generate_expr(node->assign.expr);
            snprintf(tac[tac_index].op, 8, "=");
            strcpy(tac[tac_index].arg1, val);
            tac[tac_index].arg2[0] = '\0';
            strcpy(tac[tac_index].result, node->assign.var_name);
            tac_index++;
            break;
        }

        case NODE_PRINT: {
            char* val = generate_expr(node->print_expr);
            snprintf(tac[tac_index].op, 8, "print");
            strcpy(tac[tac_index].arg1, val);
            tac[tac_index].arg2[0] = '\0';
            tac[tac_index].result[0] = '\0';
            tac_index++;
            break;
        }

        case NODE_IF: {
            char* cond = generate_expr(node->if_stmt.condition);
            char label_if[16], label_else[16], label_end[16];
            static int label_index = 0;
            snprintf(label_if, sizeof(label_if), "L%d", label_index++);
            snprintf(label_else, sizeof(label_else), "L%d", label_index++);
            snprintf(label_end, sizeof(label_end), "L%d", label_index++);

            // if cond goto label_if
            snprintf(tac[tac_index].op, 8, "ifgoto");
            strcpy(tac[tac_index].arg1, cond);
            strcpy(tac[tac_index].arg2, label_if);
            tac[tac_index].result[0] = '\0';
            tac_index++;

            // goto label_else
            snprintf(tac[tac_index].op, 8, "goto");
            strcpy(tac[tac_index].arg1, label_else);
            tac[tac_index].arg2[0] = '\0';
            tac[tac_index].result[0] = '\0';
            tac_index++;

            // label_if:
            snprintf(tac[tac_index].op, 8, "label");
            strcpy(tac[tac_index].arg1, label_if);
            tac[tac_index].arg2[0] = '\0';
            tac[tac_index].result[0] = '\0';
            tac_index++;

            generate_stmt(node->if_stmt.if_body);

            // goto label_end
            snprintf(tac[tac_index].op, 8, "goto");
            strcpy(tac[tac_index].arg1, label_end);
            tac[tac_index].arg2[0] = '\0';
            tac[tac_index].result[0] = '\0';
            tac_index++;

            // label_else:
            snprintf(tac[tac_index].op, 8, "label");
            strcpy(tac[tac_index].arg1, label_else);
            tac[tac_index].arg2[0] = '\0';
            tac[tac_index].result[0] = '\0';
            tac_index++;

            if (node->if_stmt.else_body)
                generate_stmt(node->if_stmt.else_body);

            // label_end:
            snprintf(tac[tac_index].op, 8, "label");
            strcpy(tac[tac_index].arg1, label_end);
            tac[tac_index].arg2[0] = '\0';
            tac[tac_index].result[0] = '\0';
            tac_index++;

            break;
        }

        default:
            break;
    }
}

char* generate_expr(ASTNode* node) {
    if (!node) return strdup("?");

    switch (node->type) {
        case NODE_INT: {
            char* temp = malloc(16);
            snprintf(temp, 16, "%d", node->int_value);
            return temp;
        }
        case NODE_BOOL: {
            char* temp = malloc(16);
            snprintf(temp, 16, "%d", node->int_value);
            return temp;
        }
        case NODE_VAR:
            return strdup(node->var_name);

        case NODE_BINOP: {
            char* left = generate_expr(node->binop.left);
            char* right = generate_expr(node->binop.right);
            char* temp = new_temp();

            snprintf(tac[tac_index].op, 8, "%s", node->binop.op);
            strcpy(tac[tac_index].arg1, left);
            strcpy(tac[tac_index].arg2, right);
            strcpy(tac[tac_index].result, temp);
            tac_index++;

            return temp;
        }

        default:
            return strdup("?");
    }
}

void emit_TAC_to_file(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    for (int i = 0; i < tac_index; i++) {
        if (strcmp(tac[i].op, "print") == 0) {
            fprintf(f, "print %s\n", tac[i].arg1);
        } else if (strcmp(tac[i].op, "=") == 0) {
            fprintf(f, "%s = %s\n", tac[i].result, tac[i].arg1);
        } else if (strcmp(tac[i].op, "ifgoto") == 0) {
            fprintf(f, "if %s goto %s\n", tac[i].arg1, tac[i].arg2);
        } else if (strcmp(tac[i].op, "goto") == 0) {
            fprintf(f, "goto %s\n", tac[i].arg1);
        } else if (strcmp(tac[i].op, "label") == 0) {
            fprintf(f, "%s:\n", tac[i].arg1);
        } else {
            fprintf(f, "%s = %s %s %s\n", tac[i].result, tac[i].arg1, tac[i].op, tac[i].arg2);
        }
    }
    fclose(f);
}

void generate_code(ASTNode* root, const char* filename) {
    tac_index = 0;
    temp_index = 0;
    generate_stmt(root);
    emit_TAC_to_file(filename);
}
