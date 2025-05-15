// ast.h

#ifndef AST_H
#define AST_H

typedef enum {
    NODE_INT,
    NODE_VAR,
    NODE_BINOP,
    NODE_ASSIGN,
    NODE_DECL,
    NODE_PRINT,
    NODE_IF,
    NODE_STMT_LIST
} NodeType;

typedef enum {
  TYPE_INT,
  TYPE_ERROR
} Type;

typedef struct ASTNode {
    NodeType type;

    union {
        // NODE_INT
        int int_value;

        // NODE_VAR
        char* var_name;

        // NODE_BINOP
        struct {
            char* op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binop;

        // NODE_ASSIGN
        struct {
            char* var_name;
            struct ASTNode* expr;
        } assign;

        // NODE_DECL
        struct {
            char* var_name;
            struct ASTNode* init_value; // can be NULL
        } decl;

        // NODE_PRINT
        struct ASTNode* print_expr;

        // NODE_IF
        struct {
            struct ASTNode* condition;
            struct ASTNode* if_body;
            struct ASTNode* else_body; // can be NULL
        } if_stmt;

        // NODE_STMT_LIST
        struct {
            struct ASTNode** stmts;
            int count;
            int capacity;
        } stmt_list;
    };

} ASTNode;

// Create functions
ASTNode* make_int_node(int value);
ASTNode* make_var_node(char* name);
ASTNode* make_binop_node(char* op, ASTNode* left, ASTNode* right);
ASTNode* make_assign_node(char* name, ASTNode* expr);
ASTNode* make_declaration_node(char* name, ASTNode* init);
ASTNode* make_print_node(ASTNode* expr);
ASTNode* make_if_node(ASTNode* condition, ASTNode* if_body, ASTNode* else_body);
ASTNode* make_stmt_list_node();
void     add_statement(ASTNode* list, ASTNode* stmt);
void print_ast(ASTNode* node, int indent);

#endif
