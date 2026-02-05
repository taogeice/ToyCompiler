#include "ast_nodes.h"
#include "../../common/diagnostics/diagnostic_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================== 内部辅助函数 ====================

/**
 * @brief AST节点类型名称表
 */
static const char* astNodeTypeNames[] = {
    // 基础类型
    "Expression",
    "Statement",
    "Declaration",
    "TypeSpecifier",
    "TranslationUnit",

    // 表达式子类型
    "LiteralExpr",
    "IdentifierExpr",
    "BinaryOperatorExpr",
    "UnaryOperatorExpr",
    "AssignmentExpr",
    "TernaryExpr",
    "FunctionCallExpr",
    "ArraySubscriptExpr",
    "MemberAccessExpr",
    "CastExpr",

    // 语句子类型
    "ExpressionStatement",
    "CompoundStatement",
    "IfStatement",
    "WhileStatement",
    "DoWhileStatement",
    "ForStatement",
    "ReturnStatement",
    "BreakStatement",
    "ContinueStatement",
    "SwitchStatement",
    "CaseStatement",
    "LabeledStatement",
    "GotoStatement",

    // 声明子类型
    "VariableDeclaration",
    "FunctionDeclaration",
    "StructDeclaration",
    "UnionDeclaration",
    "EnumDeclaration",
    "TypedefDeclaration",

    // 类型说明符子类型
    "BasicTypeSpecifier",
    "PointerTypeSpecifier",
    "ArrayTypeSpecifier",
    "FunctionTypeSpecifier",
    "StructTypeSpecifier",
    "UnionTypeSpecifier",
    "EnumTypeSpecifier",
    "TypedefNameSpecifier"
};

// ==================== 基础AST节点函数 ====================

/**
 * @brief 通用accept函数（占位符）
 */
static void astNodeAccept(ASTNode* self, ASTVisitor* visitor) {
    // 子类会覆盖此函数
    (void)self;
    (void)visitor;
}

/**
 * @brief 创建AST节点
 */
ASTNode* createASTNode(ASTNodeType type, SourceLocation location) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }

    node->location = location;
    node->parent = NULL;
    node->nodeType = type;
    node->accept = astNodeAccept;
    node->destroy = free;  // 默认析构函数

    return node;
}

/**
 * @brief 销毁AST节点
 */
void destroyASTNode(ASTNode* node) {
    if (node && node->destroy) {
        node->destroy(node);
    }
}

/**
 * @brief 创建翻译单元
 */
TranslationUnit* createTranslationUnit(void) {
    TranslationUnit* unit = (TranslationUnit*)malloc(sizeof(TranslationUnit));
    if (!unit) {
        return NULL;
    }

    unit->base.location = createSourceLocation(NULL, 0, 0, 0);
    unit->base.parent = NULL;
    unit->base.nodeType = AST_NODE_TRANSLATION_UNIT;
    unit->base.accept = astNodeAccept;
    unit->base.destroy = (void*)destroyTranslationUnit;

    unit->declarations = vectorCreate(sizeof(Declaration*), 16);

    return unit;
}

/**
 * @brief 销毁翻译单元
 */
void destroyTranslationUnit(TranslationUnit* unit) {
    if (!unit) {
        return;
    }

    if (unit->declarations) {
        // 销毁所有声明
        for (size_t i = 0; i < vectorSize(unit->declarations); i++) {
            Declaration** declPtr = vectorGet(unit->declarations, i);
            if (declPtr && *declPtr) {
                DESTROY_AST_NODE(*declPtr);
            }
        }
        vectorDestroy(unit->declarations, NULL);
    }

    free(unit);
}

/**
 * @brief 获取AST节点类型名称
 */
const char* astNodeTypeName(ASTNodeType type) {
    if (type >= 0 && type < sizeof(astNodeTypeNames) / sizeof(astNodeTypeNames[0])) {
        return astNodeTypeNames[type];
    }
    return "Unknown";
}

/**
 * @brief 设置节点的父节点
 */
void astNodeSetParent(ASTNode* node, ASTNode* parent) {
    if (node) {
        node->parent = parent;
    }
}

/**
 * @brief 获取节点的源位置
 */
SourceLocation astNodeGetLocation(const ASTNode* node) {
    if (node) {
        return node->location;
    }
    return createSourceLocation(NULL, 0, 0, 0);
}

// ==================== 表达式节点实现 ====================

/**
 * @brief 销毁表达式基础结构
 */
static void destroyExpression(ASTNode* node) {
    if (!node) {
        return;
    }

    Expression* expr = (Expression*)node;

    // 注意：type指针由语义分析器管理，不在这里释放
    free(expr);
}

/**
 * @brief 创建字面量表达式
 */
Expression* createLiteralExpr(Token* literal, SourceLocation location) {
    LiteralExpr* expr = (LiteralExpr*)malloc(sizeof(LiteralExpr));
    if (!expr) {
        return NULL;
    }

    // 初始化基础结构
    expr->base.base.location = location;
    expr->base.base.parent = NULL;
    expr->base.base.nodeType = AST_NODE_LITERAL_EXPR;
    expr->base.base.accept = astNodeAccept;
    expr->base.base.destroy = destroyExpression;

    expr->base.type = NULL;
    expr->base.exprKind = EXPR_LITERAL;
    expr->base.isLvalue = false;
    expr->base.isConstant = true;

    expr->literalToken = literal;

    return (Expression*)expr;
}

/**
 * @brief 创建标识符表达式
 */
Expression* createIdentifierExpr(const char* name, SourceLocation location) {
    if (!name) {
        return NULL;
    }

    IdentifierExpr* expr = (IdentifierExpr*)malloc(sizeof(IdentifierExpr));
    if (!expr) {
        return NULL;
    }

    expr->base.base.location = location;
    expr->base.base.parent = NULL;
    expr->base.base.nodeType = AST_NODE_IDENTIFIER_EXPR;
    expr->base.base.accept = astNodeAccept;
    expr->base.base.destroy = destroyExpression;

    expr->base.type = NULL;
    expr->base.exprKind = EXPR_IDENTIFIER;
    expr->base.isLvalue = true;   // 标识符通常是左值
    expr->base.isConstant = false;

    expr->name = strdup(name);
    expr->symbol = NULL;

    return (Expression*)expr;
}

/**
 * @brief 销毁标识符表达式
 */
static void destroyIdentifierExpr(ASTNode* node) {
    if (!node) {
        return;
    }

    IdentifierExpr* expr = (IdentifierExpr*)node;
    if (expr->name) {
        free(expr->name);
    }
    free(expr);
}

/**
 * @brief 创建二元运算符表达式
 */
Expression* createBinaryOperatorExpr(BinaryOperator op, Expression* left, Expression* right, SourceLocation location) {
    BinaryOperatorExpr* expr = (BinaryOperatorExpr*)malloc(sizeof(BinaryOperatorExpr));
    if (!expr) {
        return NULL;
    }

    expr->base.base.location = location;
    expr->base.base.parent = NULL;
    expr->base.base.nodeType = AST_NODE_BINARY_OPERATOR_EXPR;
    expr->base.base.accept = astNodeAccept;
    expr->base.base.destroy = destroyExpression;

    expr->base.type = NULL;
    expr->base.exprKind = EXPR_BINARY_OPERATOR;
    expr->base.isLvalue = false;
    expr->base.isConstant = false;  // 取决于操作数

    expr->op = op;
    expr->left = left;
    expr->right = right;

    // 设置父节点
    if (left) {
        left->base.parent = (ASTNode*)expr;
    }
    if (right) {
        right->base.parent = (ASTNode*)expr;
    }

    return (Expression*)expr;
}

/**
 * @brief 销毁二元运算符表达式
 */
static void destroyBinaryOperatorExpr(ASTNode* node) {
    if (!node) {
        return;
    }

    BinaryOperatorExpr* expr = (BinaryOperatorExpr*)node;
    if (expr->left) {
        DESTROY_AST_NODE(expr->left);
    }
    if (expr->right) {
        DESTROY_AST_NODE(expr->right);
    }
    free(expr);
}

/**
 * @brief 创建一元运算符表达式
 */
Expression* createUnaryOperatorExpr(UnaryOperator op, Expression* operand, bool isPrefix, SourceLocation location) {
    UnaryOperatorExpr* expr = (UnaryOperatorExpr*)malloc(sizeof(UnaryOperatorExpr));
    if (!expr) {
        return NULL;
    }

    expr->base.base.location = location;
    expr->base.base.parent = NULL;
    expr->base.base.nodeType = AST_NODE_UNARY_OPERATOR_EXPR;
    expr->base.base.accept = astNodeAccept;
    expr->base.base.destroy = destroyExpression;

    expr->base.type = NULL;
    expr->base.exprKind = EXPR_UNARY_OPERATOR;
    expr->base.isLvalue = false;  // 取决于运算符
    expr->base.isConstant = false;

    expr->op = op;
    expr->operand = operand;
    expr->isPrefix = isPrefix;

    if (operand) {
        operand->base.parent = (ASTNode*)expr;
    }

    return (Expression*)expr;
}

/**
 * @brief 销毁一元运算符表达式
 */
static void destroyUnaryOperatorExpr(ASTNode* node) {
    if (!node) {
        return;
    }

    UnaryOperatorExpr* expr = (UnaryOperatorExpr*)node;
    if (expr->operand) {
        DESTROY_AST_NODE(expr->operand);
    }
    free(expr);
}

/**
 * @brief 创建赋值表达式
 */
Expression* createAssignmentExpr(AssignmentKind kind, Expression* left, Expression* right, SourceLocation location) {
    AssignmentExpr* expr = (AssignmentExpr*)malloc(sizeof(AssignmentExpr));
    if (!expr) {
        return NULL;
    }

    expr->base.base.location = location;
    expr->base.base.parent = NULL;
    expr->base.base.nodeType = AST_NODE_ASSIGNMENT_EXPR;
    expr->base.base.accept = astNodeAccept;
    expr->base.base.destroy = destroyExpression;

    expr->base.type = NULL;
    expr->base.exprKind = EXPR_ASSIGNMENT;
    expr->base.isLvalue = false;
    expr->base.isConstant = false;

    expr->kind = kind;
    expr->left = left;
    expr->right = right;

    if (left) {
        left->base.parent = (ASTNode*)expr;
    }
    if (right) {
        right->base.parent = (ASTNode*)expr;
    }

    return (Expression*)expr;
}

/**
 * @brief 销毁赋值表达式
 */
static void destroyAssignmentExpr(ASTNode* node) {
    if (!node) {
        return;
    }

    AssignmentExpr* expr = (AssignmentExpr*)node;
    if (expr->left) {
        DESTROY_AST_NODE(expr->left);
    }
    if (expr->right) {
        DESTROY_AST_NODE(expr->right);
    }
    free(expr);
}

/**
 * @brief 创建三元表达式
 */
Expression* createTernaryExpr(Expression* condition, Expression* thenExpr, Expression* elseExpr, SourceLocation location) {
    TernaryExpr* expr = (TernaryExpr*)malloc(sizeof(TernaryExpr));
    if (!expr) {
        return NULL;
    }

    expr->base.base.location = location;
    expr->base.base.parent = NULL;
    expr->base.base.nodeType = AST_NODE_TERNARY_EXPR;
    expr->base.base.accept = astNodeAccept;
    expr->base.base.destroy = destroyExpression;

    expr->base.type = NULL;
    expr->base.exprKind = EXPR_TERNARY;
    expr->base.isLvalue = false;
    expr->base.isConstant = false;

    expr->condition = condition;
    expr->thenExpr = thenExpr;
    expr->elseExpr = elseExpr;

    if (condition) {
        condition->base.parent = (ASTNode*)expr;
    }
    if (thenExpr) {
        thenExpr->base.parent = (ASTNode*)expr;
    }
    if (elseExpr) {
        elseExpr->base.parent = (ASTNode*)expr;
    }

    return (Expression*)expr;
}

/**
 * @brief 销毁三元表达式
 */
static void destroyTernaryExpr(ASTNode* node) {
    if (!node) {
        return;
    }

    TernaryExpr* expr = (TernaryExpr*)node;
    if (expr->condition) {
        DESTROY_AST_NODE(expr->condition);
    }
    if (expr->thenExpr) {
        DESTROY_AST_NODE(expr->thenExpr);
    }
    if (expr->elseExpr) {
        DESTROY_AST_NODE(expr->elseExpr);
    }
    free(expr);
}

/**
 * @brief 创建函数调用表达式
 */
Expression* createFunctionCallExpr(Expression* callee, Vector* arguments, SourceLocation location) {
    FunctionCallExpr* expr = (FunctionCallExpr*)malloc(sizeof(FunctionCallExpr));
    if (!expr) {
        return NULL;
    }

    expr->base.base.location = location;
    expr->base.base.parent = NULL;
    expr->base.base.nodeType = AST_NODE_FUNCTION_CALL_EXPR;
    expr->base.base.accept = astNodeAccept;
    expr->base.base.destroy = destroyExpression;

    expr->base.type = NULL;
    expr->base.exprKind = EXPR_FUNCTION_CALL;
    expr->base.isLvalue = false;
    expr->base.isConstant = false;

    expr->callee = callee;
    expr->arguments = arguments;

    if (callee) {
        callee->base.parent = (ASTNode*)expr;
    }

    return (Expression*)expr;
}

/**
 * @brief 销毁函数调用表达式
 */
static void destroyFunctionCallExpr(ASTNode* node) {
    if (!node) {
        return;
    }

    FunctionCallExpr* expr = (FunctionCallExpr*)node;
    if (expr->callee) {
        DESTROY_AST_NODE(expr->callee);
    }
    if (expr->arguments) {
        for (size_t i = 0; i < vectorSize(expr->arguments); i++) {
            Expression** argPtr = vectorGet(expr->arguments, i);
            if (argPtr && *argPtr) {
                DESTROY_AST_NODE(*argPtr);
            }
        }
        vectorDestroy(expr->arguments, NULL);
    }
    free(expr);
}

/**
 * @brief 创建数组下标表达式
 */
Expression* createArraySubscriptExpr(Expression* array, Expression* index, SourceLocation location) {
    ArraySubscriptExpr* expr = (ArraySubscriptExpr*)malloc(sizeof(ArraySubscriptExpr));
    if (!expr) {
        return NULL;
    }

    expr->base.base.location = location;
    expr->base.base.parent = NULL;
    expr->base.base.nodeType = AST_NODE_ARRAY_SUBSCRIPT_EXPR;
    expr->base.base.accept = astNodeAccept;
    expr->base.base.destroy = destroyExpression;

    expr->base.type = NULL;
    expr->base.exprKind = EXPR_ARRAY_SUBSCRIPT;
    expr->base.isLvalue = true;   // 数组元素是左值
    expr->base.isConstant = false;

    expr->array = array;
    expr->index = index;

    if (array) {
        array->base.parent = (ASTNode*)expr;
    }
    if (index) {
        index->base.parent = (ASTNode*)expr;
    }

    return (Expression*)expr;
}

/**
 * @brief 销毁数组下标表达式
 */
static void destroyArraySubscriptExpr(ASTNode* node) {
    if (!node) {
        return;
    }

    ArraySubscriptExpr* expr = (ArraySubscriptExpr*)node;
    if (expr->array) {
        DESTROY_AST_NODE(expr->array);
    }
    if (expr->index) {
        DESTROY_AST_NODE(expr->index);
    }
    free(expr);
}

/**
 * @brief 创建成员访问表达式
 */
Expression* createMemberAccessExpr(Expression* baseExpr, const char* memberName, bool isArrow, SourceLocation location) {
    if (!memberName) {
        return NULL;
    }

    MemberAccessExpr* expr = (MemberAccessExpr*)malloc(sizeof(MemberAccessExpr));
    if (!expr) {
        return NULL;
    }

    expr->base.base.location = location;
    expr->base.base.parent = NULL;
    expr->base.base.nodeType = AST_NODE_MEMBER_ACCESS_EXPR;
    expr->base.base.accept = astNodeAccept;
    expr->base.base.destroy = destroyExpression;

    expr->base.type = NULL;
    expr->base.exprKind = EXPR_MEMBER_ACCESS;
    expr->base.isLvalue = true;   // 成员访问是左值
    expr->base.isConstant = false;

    expr->baseExpr = baseExpr;
    expr->memberName = strdup(memberName);
    expr->isArrow = isArrow;

    if (baseExpr) {
        baseExpr->base.parent = (ASTNode*)expr;
    }

    return (Expression*)expr;
}

/**
 * @brief 销毁成员访问表达式
 */
static void destroyMemberAccessExpr(ASTNode* node) {
    if (!node) {
        return;
    }

    MemberAccessExpr* expr = (MemberAccessExpr*)node;
    if (expr->baseExpr) {
        DESTROY_AST_NODE(expr->baseExpr);
    }
    if (expr->memberName) {
        free(expr->memberName);
    }
    free(expr);
}

/**
 * @brief 创建类型转换表达式
 */
Expression* createCastExpr(TypeSpecifier* targetType, Expression* operand, SourceLocation location) {
    CastExpr* expr = (CastExpr*)malloc(sizeof(CastExpr));
    if (!expr) {
        return NULL;
    }

    expr->base.base.location = location;
    expr->base.base.parent = NULL;
    expr->base.base.nodeType = AST_NODE_CAST_EXPR;
    expr->base.base.accept = astNodeAccept;
    expr->base.base.destroy = destroyExpression;

    expr->base.type = NULL;
    expr->base.exprKind = EXPR_CAST;
    expr->base.isLvalue = false;
    expr->base.isConstant = false;

    expr->targetType = targetType;
    expr->operand = operand;

    if (operand) {
        operand->base.parent = (ASTNode*)expr;
    }

    return (Expression*)expr;
}

/**
 * @brief 销毁类型转换表达式
 */
static void destroyCastExpr(ASTNode* node) {
    if (!node) {
        return;
    }

    CastExpr* expr = (CastExpr*)node;
    // 注意：targetType 由类型系统管理，不在这里释放
    if (expr->operand) {
        DESTROY_AST_NODE(expr->operand);
    }
    free(expr);
}

// ==================== 运算符字符串转换 ====================

const char* binaryOperatorToString(BinaryOperator op) {
    switch (op) {
        case BINOP_ADD: return "+";
        case BINOP_SUB: return "-";
        case BINOP_MUL: return "*";
        case BINOP_DIV: return "/";
        case BINOP_MOD: return "%";
        case BINOP_EQ: return "==";
        case BINOP_NE: return "!=";
        case BINOP_LT: return "<";
        case BINOP_LE: return "<=";
        case BINOP_GT: return ">";
        case BINOP_GE: return ">=";
        case BINOP_LOGICAL_AND: return "&&";
        case BINOP_LOGICAL_OR: return "||";
        case BINOP_BITWISE_AND: return "&";
        case BINOP_BITWISE_OR: return "|";
        case BINOP_BITWISE_XOR: return "^";
        case BINOP_LEFT_SHIFT: return "<<";
        case BINOP_RIGHT_SHIFT: return ">>";
        case BINOP_COMMA: return ",";
        default: return "?";
    }
}

const char* unaryOperatorToString(UnaryOperator op) {
    switch (op) {
        case UNOP_POSTFIX_INC: return "++";
        case UNOP_POSTFIX_DEC: return "--";
        case UNOP_PREFIX_INC: return "++";
        case UNOP_PREFIX_DEC: return "--";
        case UNOP_PLUS: return "+";
        case UNOP_MINUS: return "-";
        case UNOP_BITWISE_NOT: return "~";
        case UNOP_LOGICAL_NOT: return "!";
        case UNOP_DEREF: return "*";
        case UNOP_ADDRESS_OF: return "&";
        case UNOP_SIZEOF: return "sizeof";
        default: return "?";
    }
}

const char* assignmentKindToString(AssignmentKind kind) {
    switch (kind) {
        case ASSIGN_SIMPLE: return "=";
        case ASSIGN_ADD: return "+=";
        case ASSIGN_SUB: return "-=";
        case ASSIGN_MUL: return "*=";
        case ASSIGN_DIV: return "/=";
        case ASSIGN_MOD: return "%=";
        case ASSIGN_SHL: return "<<=";
        case ASSIGN_SHR: return ">>=";
        case ASSIGN_AND: return "&=";
        case ASSIGN_OR: return "|=";
        case ASSIGN_XOR: return "^=";
        default: return "?";
    }
}

// ==================== 语句节点实现 ====================

/**
 * @brief 销毁语句基础结构
 */
static void destroyStatement(ASTNode* node) {
    if (!node) {
        return;
    }
    free(node);
}

/**
 * @brief 创建表达式语句
 */
Statement* createExpressionStatement(Expression* expression, SourceLocation location) {
    ExpressionStatement* stmt = (ExpressionStatement*)malloc(sizeof(ExpressionStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_EXPRESSION_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_EXPRESSION;

    stmt->expression = expression;
    if (expression) {
        expression->base.parent = (ASTNode*)stmt;
    }

    return (Statement*)stmt;
}

/**
 * @brief 销毁表达式语句
 */
static void destroyExpressionStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    ExpressionStatement* stmt = (ExpressionStatement*)node;
    if (stmt->expression) {
        DESTROY_AST_NODE(stmt->expression);
    }
    free(stmt);
}

/**
 * @brief 创建复合语句
 */
Statement* createCompoundStatement(SourceLocation location) {
    CompoundStatement* stmt = (CompoundStatement*)malloc(sizeof(CompoundStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_COMPOUND_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_COMPOUND;

    stmt->declarations = vectorCreate(sizeof(Declaration*), 8);
    stmt->statements = vectorCreate(sizeof(Statement*), 8);

    return (Statement*)stmt;
}

/**
 * @brief 销毁复合语句
 */
static void destroyCompoundStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    CompoundStatement* stmt = (CompoundStatement*)node;

    if (stmt->declarations) {
        for (size_t i = 0; i < vectorSize(stmt->declarations); i++) {
            Declaration** declPtr = vectorGet(stmt->declarations, i);
            if (declPtr && *declPtr) {
                DESTROY_AST_NODE(*declPtr);
            }
        }
        vectorDestroy(stmt->declarations, NULL);
    }

    if (stmt->statements) {
        for (size_t i = 0; i < vectorSize(stmt->statements); i++) {
            Statement** stmtPtr = vectorGet(stmt->statements, i);
            if (stmtPtr && *stmtPtr) {
                DESTROY_AST_NODE(*stmtPtr);
            }
        }
        vectorDestroy(stmt->statements, NULL);
    }

    free(stmt);
}

/**
 * @brief 创建if语句
 */
Statement* createIfStatement(Expression* condition, Statement* thenStmt, Statement* elseStmt, SourceLocation location) {
    IfStatement* stmt = (IfStatement*)malloc(sizeof(IfStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_IF_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_IF;

    stmt->condition = condition;
    stmt->thenStmt = thenStmt;
    stmt->elseStmt = elseStmt;

    if (condition) {
        condition->base.parent = (ASTNode*)stmt;
    }
    if (thenStmt) {
        thenStmt->base.parent = (ASTNode*)stmt;
    }
    if (elseStmt) {
        elseStmt->base.parent = (ASTNode*)stmt;
    }

    return (Statement*)stmt;
}

/**
 * @brief 销毁if语句
 */
static void destroyIfStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    IfStatement* stmt = (IfStatement*)node;
    if (stmt->condition) {
        DESTROY_AST_NODE(stmt->condition);
    }
    if (stmt->thenStmt) {
        DESTROY_AST_NODE(stmt->thenStmt);
    }
    if (stmt->elseStmt) {
        DESTROY_AST_NODE(stmt->elseStmt);
    }
    free(stmt);
}

/**
 * @brief 创建while语句
 */
Statement* createWhileStatement(Expression* condition, Statement* body, SourceLocation location) {
    WhileStatement* stmt = (WhileStatement*)malloc(sizeof(WhileStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_WHILE_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_WHILE;

    stmt->condition = condition;
    stmt->body = body;

    if (condition) {
        condition->base.parent = (ASTNode*)stmt;
    }
    if (body) {
        body->base.parent = (ASTNode*)stmt;
    }

    return (Statement*)stmt;
}

/**
 * @brief 销毁while语句
 */
static void destroyWhileStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    WhileStatement* stmt = (WhileStatement*)node;
    if (stmt->condition) {
        DESTROY_AST_NODE(stmt->condition);
    }
    if (stmt->body) {
        DESTROY_AST_NODE(stmt->body);
    }
    free(stmt);
}

/**
 * @brief 创建do-while语句
 */
Statement* createDoWhileStatement(Statement* body, Expression* condition, SourceLocation location) {
    DoWhileStatement* stmt = (DoWhileStatement*)malloc(sizeof(DoWhileStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_DO_WHILE_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_DO_WHILE;

    stmt->body = body;
    stmt->condition = condition;

    if (body) {
        body->base.parent = (ASTNode*)stmt;
    }
    if (condition) {
        condition->base.parent = (ASTNode*)stmt;
    }

    return (Statement*)stmt;
}

/**
 * @brief 销毁do-while语句
 */
static void destroyDoWhileStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    DoWhileStatement* stmt = (DoWhileStatement*)node;
    if (stmt->body) {
        DESTROY_AST_NODE(stmt->body);
    }
    if (stmt->condition) {
        DESTROY_AST_NODE(stmt->condition);
    }
    free(stmt);
}

/**
 * @brief 创建for语句
 */
Statement* createForStatement(Expression* init, Expression* condition, Expression* increment, Statement* body, SourceLocation location) {
    ForStatement* stmt = (ForStatement*)malloc(sizeof(ForStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_FOR_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_FOR;

    stmt->init = init;
    stmt->condition = condition;
    stmt->increment = increment;
    stmt->body = body;

    if (init) {
        init->base.parent = (ASTNode*)stmt;
    }
    if (condition) {
        condition->base.parent = (ASTNode*)stmt;
    }
    if (increment) {
        increment->base.parent = (ASTNode*)stmt;
    }
    if (body) {
        body->base.parent = (ASTNode*)stmt;
    }

    return (Statement*)stmt;
}

/**
 * @brief 销毁for语句
 */
static void destroyForStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    ForStatement* stmt = (ForStatement*)node;
    if (stmt->init) {
        DESTROY_AST_NODE(stmt->init);
    }
    if (stmt->condition) {
        DESTROY_AST_NODE(stmt->condition);
    }
    if (stmt->increment) {
        DESTROY_AST_NODE(stmt->increment);
    }
    if (stmt->body) {
        DESTROY_AST_NODE(stmt->body);
    }
    free(stmt);
}

/**
 * @brief 创建return语句
 */
Statement* createReturnStatement(Expression* returnValue, SourceLocation location) {
    ReturnStatement* stmt = (ReturnStatement*)malloc(sizeof(ReturnStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_RETURN_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_RETURN;

    stmt->returnValue = returnValue;
    if (returnValue) {
        returnValue->base.parent = (ASTNode*)stmt;
    }

    return (Statement*)stmt;
}

/**
 * @brief 销毁return语句
 */
static void destroyReturnStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    ReturnStatement* stmt = (ReturnStatement*)node;
    if (stmt->returnValue) {
        DESTROY_AST_NODE(stmt->returnValue);
    }
    free(stmt);
}

/**
 * @brief 创建break语句
 */
Statement* createBreakStatement(SourceLocation location) {
    BreakStatement* stmt = (BreakStatement*)malloc(sizeof(BreakStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_BREAK_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_BREAK;

    return (Statement*)stmt;
}

/**
 * @brief 创建continue语句
 */
Statement* createContinueStatement(SourceLocation location) {
    ContinueStatement* stmt = (ContinueStatement*)malloc(sizeof(ContinueStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_CONTINUE_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_CONTINUE;

    return (Statement*)stmt;
}

/**
 * @brief 创建switch语句
 */
Statement* createSwitchStatement(Expression* condition, Vector* cases, SourceLocation location) {
    SwitchStatement* stmt = (SwitchStatement*)malloc(sizeof(SwitchStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_SWITCH_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_SWITCH;

    stmt->condition = condition;
    stmt->cases = cases;

    if (condition) {
        condition->base.parent = (ASTNode*)stmt;
    }

    return (Statement*)stmt;
}

/**
 * @brief 销毁switch语句
 */
static void destroySwitchStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    SwitchStatement* stmt = (SwitchStatement*)node;
    if (stmt->condition) {
        DESTROY_AST_NODE(stmt->condition);
    }
    if (stmt->cases) {
        for (size_t i = 0; i < vectorSize(stmt->cases); i++) {
            Statement** casePtr = vectorGet(stmt->cases, i);
            if (casePtr && *casePtr) {
                DESTROY_AST_NODE(*casePtr);
            }
        }
        vectorDestroy(stmt->cases, NULL);
    }
    free(stmt);
}

/**
 * @brief 创建case/default语句
 */
Statement* createCaseStatement(CaseKind kind, Expression* value, Statement* statement, SourceLocation location) {
    CaseStatement* stmt = (CaseStatement*)malloc(sizeof(CaseStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_CASE_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_CASE;

    stmt->kind = kind;
    stmt->value = value;
    stmt->statement = statement;

    if (value) {
        value->base.parent = (ASTNode*)stmt;
    }
    if (statement) {
        statement->base.parent = (ASTNode*)stmt;
    }

    return (Statement*)stmt;
}

/**
 * @brief 销毁case语句
 */
static void destroyCaseStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    CaseStatement* stmt = (CaseStatement*)node;
    if (stmt->value) {
        DESTROY_AST_NODE(stmt->value);
    }
    if (stmt->statement) {
        DESTROY_AST_NODE(stmt->statement);
    }
    free(stmt);
}

/**
 * @brief 创建标签语句
 */
Statement* createLabeledStatement(const char* labelName, Statement* statement, SourceLocation location) {
    if (!labelName) {
        return NULL;
    }

    LabeledStatement* stmt = (LabeledStatement*)malloc(sizeof(LabeledStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_LABELED_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_LABELED;

    stmt->labelName = strdup(labelName);
    stmt->statement = statement;

    if (statement) {
        statement->base.parent = (ASTNode*)stmt;
    }

    return (Statement*)stmt;
}

/**
 * @brief 销毁标签语句
 */
static void destroyLabeledStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    LabeledStatement* stmt = (LabeledStatement*)node;
    if (stmt->labelName) {
        free(stmt->labelName);
    }
    if (stmt->statement) {
        DESTROY_AST_NODE(stmt->statement);
    }
    free(stmt);
}

/**
 * @brief 创建goto语句
 */
Statement* createGotoStatement(const char* labelName, SourceLocation location) {
    if (!labelName) {
        return NULL;
    }

    GotoStatement* stmt = (GotoStatement*)malloc(sizeof(GotoStatement));
    if (!stmt) {
        return NULL;
    }

    stmt->base.base.location = location;
    stmt->base.base.parent = NULL;
    stmt->base.base.nodeType = AST_NODE_GOTO_STATEMENT;
    stmt->base.base.accept = astNodeAccept;
    stmt->base.base.destroy = destroyStatement;

    stmt->base.stmtKind = STMT_GOTO;

    stmt->labelName = strdup(labelName);

    return (Statement*)stmt;
}

/**
 * @brief 销毁goto语句
 */
static void destroyGotoStatement(ASTNode* node) {
    if (!node) {
        return;
    }

    GotoStatement* stmt = (GotoStatement*)node;
    if (stmt->labelName) {
        free(stmt->labelName);
    }
    free(stmt);
}

// ==================== 声明节点实现 ====================

/**
 * @brief 销毁声明基础结构
 */
static void destroyDeclaration(ASTNode* node) {
    if (!node) {
        return;
    }

    Declaration* decl = (Declaration*)node;
    if (decl->name) {
        free(decl->name);
    }
    // symbol 由符号表管理，不在这里释放
    free(decl);
}

/**
 * @brief 创建变量声明
 */
Declaration* createVariableDeclaration(const char* name, TypeSpecifier* type, Expression* initializer, SourceLocation location) {
    if (!name) {
        return NULL;
    }

    VariableDeclaration* decl = (VariableDeclaration*)malloc(sizeof(VariableDeclaration));
    if (!decl) {
        return NULL;
    }

    decl->base.base.location = location;
    decl->base.base.parent = NULL;
    decl->base.base.nodeType = AST_NODE_VARIABLE_DECLARATION;
    decl->base.base.accept = astNodeAccept;
    decl->base.base.destroy = destroyDeclaration;

    decl->base.name = strdup(name);
    decl->base.declKind = DECL_VARIABLE;
    decl->base.symbol = NULL;
    decl->base.storageClass = STORAGE_CLASS_NONE;

    decl->type = type;
    decl->initializer = initializer;
    decl->isConst = false;
    decl->isVolatile = false;

    if (initializer) {
        initializer->base.parent = (ASTNode*)decl;
    }

    return (Declaration*)decl;
}

/**
 * @brief 销毁变量声明
 */
static void destroyVariableDeclaration(ASTNode* node) {
    if (!node) {
        return;
    }

    VariableDeclaration* decl = (VariableDeclaration*)node;
    // type 由类型系统管理，不在这里释放
    if (decl->initializer) {
        DESTROY_AST_NODE(decl->initializer);
    }
    if (decl->base.name) {
        free(decl->base.name);
    }
    free(decl);
}

/**
 * @brief 创建函数声明
 */
Declaration* createFunctionDeclaration(const char* name, TypeSpecifier* returnType, Vector* parameters, CompoundStatement* body, SourceLocation location) {
    if (!name) {
        return NULL;
    }

    FunctionDeclaration* decl = (FunctionDeclaration*)malloc(sizeof(FunctionDeclaration));
    if (!decl) {
        return NULL;
    }

    decl->base.base.location = location;
    decl->base.base.parent = NULL;
    decl->base.base.nodeType = AST_NODE_FUNCTION_DECLARATION;
    decl->base.base.accept = astNodeAccept;
    decl->base.base.destroy = destroyDeclaration;

    decl->base.name = strdup(name);
    decl->base.declKind = DECL_FUNCTION;
    decl->base.symbol = NULL;
    decl->base.storageClass = STORAGE_CLASS_NONE;

    decl->returnType = returnType;
    decl->parameters = parameters;
    decl->body = body;
    decl->isInline = false;
    decl->isNoreturn = false;

    if (body) {
        body->base.base.parent = (ASTNode*)decl;
    }

    return (Declaration*)decl;
}

/**
 * @brief 销毁函数声明
 */
static void destroyFunctionDeclaration(ASTNode* node) {
    if (!node) {
        return;
    }

    FunctionDeclaration* decl = (FunctionDeclaration*)node;
    if (decl->base.name) {
        free(decl->base.name);
    }
    // returnType 由类型系统管理
    if (decl->parameters) {
        for (size_t i = 0; i < vectorSize(decl->parameters); i++) {
            Declaration** paramPtr = vectorGet(decl->parameters, i);
            if (paramPtr && *paramPtr) {
                DESTROY_AST_NODE(*paramPtr);
            }
        }
        vectorDestroy(decl->parameters, NULL);
    }
    if (decl->body) {
        DESTROY_AST_NODE(decl->body);
    }
    free(decl);
}

/**
 * @brief 创建结构体声明
 */
Declaration* createStructDeclaration(const char* name, Vector* members, SourceLocation location) {
    StructDeclaration* decl = (StructDeclaration*)malloc(sizeof(StructDeclaration));
    if (!decl) {
        return NULL;
    }

    decl->base.base.location = location;
    decl->base.base.parent = NULL;
    decl->base.base.nodeType = AST_NODE_STRUCT_DECLARATION;
    decl->base.base.accept = astNodeAccept;
    decl->base.base.destroy = destroyDeclaration;

    decl->base.name = name ? strdup(name) : NULL;
    decl->base.declKind = DECL_STRUCT;
    decl->base.symbol = NULL;
    decl->base.storageClass = STORAGE_CLASS_NONE;

    decl->members = members;
    decl->isPacked = false;

    return (Declaration*)decl;
}

/**
 * @brief 销毁结构体声明
 */
static void destroyStructDeclaration(ASTNode* node) {
    if (!node) {
        return;
    }

    StructDeclaration* decl = (StructDeclaration*)node;
    if (decl->base.name) {
        free(decl->base.name);
    }
    if (decl->members) {
        for (size_t i = 0; i < vectorSize(decl->members); i++) {
            Declaration** memberPtr = vectorGet(decl->members, i);
            if (memberPtr && *memberPtr) {
                DESTROY_AST_NODE(*memberPtr);
            }
        }
        vectorDestroy(decl->members, NULL);
    }
    free(decl);
}

/**
 * @brief 创建联合体声明
 */
Declaration* createUnionDeclaration(const char* name, Vector* members, SourceLocation location) {
    UnionDeclaration* decl = (UnionDeclaration*)malloc(sizeof(UnionDeclaration));
    if (!decl) {
        return NULL;
    }

    decl->base.base.location = location;
    decl->base.base.parent = NULL;
    decl->base.base.nodeType = AST_NODE_UNION_DECLARATION;
    decl->base.base.accept = astNodeAccept;
    decl->base.base.destroy = destroyDeclaration;

    decl->base.name = name ? strdup(name) : NULL;
    decl->base.declKind = DECL_UNION;
    decl->base.symbol = NULL;
    decl->base.storageClass = STORAGE_CLASS_NONE;

    decl->members = members;

    return (Declaration*)decl;
}

/**
 * @brief 销毁联合体声明
 */
static void destroyUnionDeclaration(ASTNode* node) {
    if (!node) {
        return;
    }

    UnionDeclaration* decl = (UnionDeclaration*)node;
    if (decl->base.name) {
        free(decl->base.name);
    }
    if (decl->members) {
        for (size_t i = 0; i < vectorSize(decl->members); i++) {
            Declaration** memberPtr = vectorGet(decl->members, i);
            if (memberPtr && *memberPtr) {
                DESTROY_AST_NODE(*memberPtr);
            }
        }
        vectorDestroy(decl->members, NULL);
    }
    free(decl);
}

/**
 * @brief 创建枚举声明
 */
Declaration* createEnumDeclaration(const char* name, Vector* constants, SourceLocation location) {
    EnumDeclaration* decl = (EnumDeclaration*)malloc(sizeof(EnumDeclaration));
    if (!decl) {
        return NULL;
    }

    decl->base.base.location = location;
    decl->base.base.parent = NULL;
    decl->base.base.nodeType = AST_NODE_ENUM_DECLARATION;
    decl->base.base.accept = astNodeAccept;
    decl->base.base.destroy = destroyDeclaration;

    decl->base.name = name ? strdup(name) : NULL;
    decl->base.declKind = DECL_ENUM;
    decl->base.symbol = NULL;
    decl->base.storageClass = STORAGE_CLASS_NONE;

    decl->constants = constants;
    decl->underlyingType = NULL;  // 由语义分析器设置

    return (Declaration*)decl;
}

/**
 * @brief 销毁枚举声明
 */
static void destroyEnumDeclaration(ASTNode* node) {
    if (!node) {
        return;
    }

    EnumDeclaration* decl = (EnumDeclaration*)node;
    if (decl->base.name) {
        free(decl->base.name);
    }
    if (decl->constants) {
        for (size_t i = 0; i < vectorSize(decl->constants); i++) {
            EnumConstant* constant = (EnumConstant*)vectorGet(decl->constants, i);
            if (constant) {
                if (constant->name) {
                    free(constant->name);
                }
                if (constant->value) {
                    DESTROY_AST_NODE(constant->value);
                }
            }
        }
        vectorDestroy(decl->constants, free);
    }
    free(decl);
}

/**
 * @brief 创建typedef声明
 */
Declaration* createTypedefDeclaration(const char* name, TypeSpecifier* aliasedType, SourceLocation location) {
    if (!name) {
        return NULL;
    }

    TypedefDeclaration* decl = (TypedefDeclaration*)malloc(sizeof(TypedefDeclaration));
    if (!decl) {
        return NULL;
    }

    decl->base.base.location = location;
    decl->base.base.parent = NULL;
    decl->base.base.nodeType = AST_NODE_TYPEDEF_DECLARATION;
    decl->base.base.accept = astNodeAccept;
    decl->base.base.destroy = destroyDeclaration;

    decl->base.name = strdup(name);
    decl->base.declKind = DECL_TYPEDEF;
    decl->base.symbol = NULL;
    decl->base.storageClass = STORAGE_CLASS_NONE;

    decl->aliasedType = aliasedType;

    return (Declaration*)decl;
}

/**
 * @brief 销毁typedef声明
 */
static void destroyTypedefDeclaration(ASTNode* node) {
    if (!node) {
        return;
    }

    TypedefDeclaration* decl = (TypedefDeclaration*)node;
    if (decl->base.name) {
        free(decl->base.name);
    }
    // aliasedType 由类型系统管理
    free(decl);
}

// ==================== 类型说明符节点实现 ====================

/**
 * @brief 销毁类型说明符基础结构
 */
static void destroyTypeSpecifier(ASTNode* node) {
    if (!node) {
        return;
    }
    free(node);
}

/**
 * @brief 创建基础类型说明符
 */
TypeSpecifier* createBasicTypeSpecifier(BasicTypeKind kind, SourceLocation location) {
    BasicTypeSpecifier* typeSpec = (BasicTypeSpecifier*)malloc(sizeof(BasicTypeSpecifier));
    if (!typeSpec) {
        return NULL;
    }

    typeSpec->base.base.location = location;
    typeSpec->base.base.parent = NULL;
    typeSpec->base.base.nodeType = AST_NODE_BASIC_TYPE_SPECIFIER;
    typeSpec->base.base.accept = astNodeAccept;
    typeSpec->base.base.destroy = destroyTypeSpecifier;

    typeSpec->base.typeKind = TYPE_SPECIFIER_BASIC;
    typeSpec->base.isConst = false;
    typeSpec->base.isVolatile = false;

    typeSpec->kind = kind;
    typeSpec->isLong = false;
    typeSpec->isShort = false;
    typeSpec->isSigned = false;
    typeSpec->isUnsigned = false;

    return (TypeSpecifier*)typeSpec;
}

/**
 * @brief 创建指针类型说明符
 */
TypeSpecifier* createPointerTypeSpecifier(TypeSpecifier* baseType, SourceLocation location) {
    PointerTypeSpecifier* typeSpec = (PointerTypeSpecifier*)malloc(sizeof(PointerTypeSpecifier));
    if (!typeSpec) {
        return NULL;
    }

    typeSpec->base.base.location = location;
    typeSpec->base.base.parent = NULL;
    typeSpec->base.base.nodeType = AST_NODE_POINTER_TYPE_SPECIFIER;
    typeSpec->base.base.accept = astNodeAccept;
    typeSpec->base.base.destroy = destroyTypeSpecifier;

    typeSpec->base.typeKind = TYPE_SPECIFIER_POINTER;
    typeSpec->base.isConst = false;
    typeSpec->base.isVolatile = false;

    typeSpec->baseType = baseType;

    return (TypeSpecifier*)typeSpec;
}

/**
 * @brief 销毁指针类型说明符
 */
static void destroyPointerTypeSpecifier(ASTNode* node) {
    if (!node) {
        return;
    }

    PointerTypeSpecifier* typeSpec = (PointerTypeSpecifier*)node;
    // baseType 由类型系统管理，不在这里释放
    free(typeSpec);
}

/**
 * @brief 创建数组类型说明符
 */
TypeSpecifier* createArrayTypeSpecifier(TypeSpecifier* elementType, Expression* size, SourceLocation location) {
    ArrayTypeSpecifier* typeSpec = (ArrayTypeSpecifier*)malloc(sizeof(ArrayTypeSpecifier));
    if (!typeSpec) {
        return NULL;
    }

    typeSpec->base.base.location = location;
    typeSpec->base.base.parent = NULL;
    typeSpec->base.base.nodeType = AST_NODE_ARRAY_TYPE_SPECIFIER;
    typeSpec->base.base.accept = astNodeAccept;
    typeSpec->base.base.destroy = destroyTypeSpecifier;

    typeSpec->base.typeKind = TYPE_SPECIFIER_ARRAY;
    typeSpec->base.isConst = false;
    typeSpec->base.isVolatile = false;

    typeSpec->elementType = elementType;
    typeSpec->size = size;
    typeSpec->isVariableLength = false;  // C99 VLA

    if (size) {
        size->base.parent = (ASTNode*)typeSpec;
    }

    return (TypeSpecifier*)typeSpec;
}

/**
 * @brief 销毁数组类型说明符
 */
static void destroyArrayTypeSpecifier(ASTNode* node) {
    if (!node) {
        return;
    }

    ArrayTypeSpecifier* typeSpec = (ArrayTypeSpecifier*)node;
    // elementType 由类型系统管理
    if (typeSpec->size) {
        DESTROY_AST_NODE(typeSpec->size);
    }
    free(typeSpec);
}

/**
 * @brief 创建函数类型说明符
 */
TypeSpecifier* createFunctionTypeSpecifier(TypeSpecifier* returnType, Vector* parameterTypes, bool isVariadic, SourceLocation location) {
    FunctionTypeSpecifier* typeSpec = (FunctionTypeSpecifier*)malloc(sizeof(FunctionTypeSpecifier));
    if (!typeSpec) {
        return NULL;
    }

    typeSpec->base.base.location = location;
    typeSpec->base.base.parent = NULL;
    typeSpec->base.base.nodeType = AST_NODE_FUNCTION_TYPE_SPECIFIER;
    typeSpec->base.base.accept = astNodeAccept;
    typeSpec->base.base.destroy = destroyTypeSpecifier;

    typeSpec->base.typeKind = TYPE_SPECIFIER_FUNCTION;
    typeSpec->base.isConst = false;
    typeSpec->base.isVolatile = false;

    typeSpec->returnType = returnType;
    typeSpec->parameterTypes = parameterTypes;
    typeSpec->isVariadic = isVariadic;

    return (TypeSpecifier*)typeSpec;
}

/**
 * @brief 销毁函数类型说明符
 */
static void destroyFunctionTypeSpecifier(ASTNode* node) {
    if (!node) {
        return;
    }

    FunctionTypeSpecifier* typeSpec = (FunctionTypeSpecifier*)node;
    // returnType 和 parameterTypes 由类型系统管理
    free(typeSpec);
}

/**
 * @brief 创建结构体类型说明符
 */
TypeSpecifier* createStructTypeSpecifier(const char* name, StructDeclaration* declaration, SourceLocation location) {
    StructTypeSpecifier* typeSpec = (StructTypeSpecifier*)malloc(sizeof(StructTypeSpecifier));
    if (!typeSpec) {
        return NULL;
    }

    typeSpec->base.base.location = location;
    typeSpec->base.base.parent = NULL;
    typeSpec->base.base.nodeType = AST_NODE_STRUCT_TYPE_SPECIFIER;
    typeSpec->base.base.accept = astNodeAccept;
    typeSpec->base.base.destroy = destroyTypeSpecifier;

    typeSpec->base.typeKind = TYPE_SPECIFIER_STRUCT;
    typeSpec->base.isConst = false;
    typeSpec->base.isVolatile = false;

    typeSpec->name = name ? strdup(name) : NULL;
    typeSpec->declaration = declaration;

    return (TypeSpecifier*)typeSpec;
}

/**
 * @brief 销毁结构体类型说明符
 */
static void destroyStructTypeSpecifier(ASTNode* node) {
    if (!node) {
        return;
    }

    StructTypeSpecifier* typeSpec = (StructTypeSpecifier*)node;
    if (typeSpec->name) {
        free(typeSpec->name);
    }
    // declaration 由符号表管理
    free(typeSpec);
}

/**
 * @brief 创建联合体类型说明符
 */
TypeSpecifier* createUnionTypeSpecifier(const char* name, UnionDeclaration* declaration, SourceLocation location) {
    UnionTypeSpecifier* typeSpec = (UnionTypeSpecifier*)malloc(sizeof(UnionTypeSpecifier));
    if (!typeSpec) {
        return NULL;
    }

    typeSpec->base.base.location = location;
    typeSpec->base.base.parent = NULL;
    typeSpec->base.base.nodeType = AST_NODE_UNION_TYPE_SPECIFIER;
    typeSpec->base.base.accept = astNodeAccept;
    typeSpec->base.base.destroy = destroyTypeSpecifier;

    typeSpec->base.typeKind = TYPE_SPECIFIER_UNION;
    typeSpec->base.isConst = false;
    typeSpec->base.isVolatile = false;

    typeSpec->name = name ? strdup(name) : NULL;
    typeSpec->declaration = declaration;

    return (TypeSpecifier*)typeSpec;
}

/**
 * @brief 销毁联合体类型说明符
 */
static void destroyUnionTypeSpecifier(ASTNode* node) {
    if (!node) {
        return;
    }

    UnionTypeSpecifier* typeSpec = (UnionTypeSpecifier*)node;
    if (typeSpec->name) {
        free(typeSpec->name);
    }
    free(typeSpec);
}

/**
 * @brief 创建枚举类型说明符
 */
TypeSpecifier* createEnumTypeSpecifier(const char* name, EnumDeclaration* declaration, SourceLocation location) {
    EnumTypeSpecifier* typeSpec = (EnumTypeSpecifier*)malloc(sizeof(EnumTypeSpecifier));
    if (!typeSpec) {
        return NULL;
    }

    typeSpec->base.base.location = location;
    typeSpec->base.base.parent = NULL;
    typeSpec->base.base.nodeType = AST_NODE_ENUM_TYPE_SPECIFIER;
    typeSpec->base.base.accept = astNodeAccept;
    typeSpec->base.base.destroy = destroyTypeSpecifier;

    typeSpec->base.typeKind = TYPE_SPECIFIER_ENUM;
    typeSpec->base.isConst = false;
    typeSpec->base.isVolatile = false;

    typeSpec->name = name ? strdup(name) : NULL;
    typeSpec->declaration = declaration;

    return (TypeSpecifier*)typeSpec;
}

/**
 * @brief 销毁枚举类型说明符
 */
static void destroyEnumTypeSpecifier(ASTNode* node) {
    if (!node) {
        return;
    }

    EnumTypeSpecifier* typeSpec = (EnumTypeSpecifier*)node;
    if (typeSpec->name) {
        free(typeSpec->name);
    }
    free(typeSpec);
}

/**
 * @brief 创建typedef名称类型说明符
 */
TypeSpecifier* createTypedefNameSpecifier(const char* typedefName, SourceLocation location) {
    if (!typedefName) {
        return NULL;
    }

    TypedefNameSpecifier* typeSpec = (TypedefNameSpecifier*)malloc(sizeof(TypedefNameSpecifier));
    if (!typeSpec) {
        return NULL;
    }

    typeSpec->base.base.location = location;
    typeSpec->base.base.parent = NULL;
    typeSpec->base.base.nodeType = AST_NODE_TYPEDEF_NAME_SPECIFIER;
    typeSpec->base.base.accept = astNodeAccept;
    typeSpec->base.base.destroy = destroyTypeSpecifier;

    typeSpec->base.typeKind = TYPE_SPECIFIER_TYPEDEF_NAME;
    typeSpec->base.isConst = false;
    typeSpec->base.isVolatile = false;

    typeSpec->typedefName = strdup(typedefName);

    return (TypeSpecifier*)typeSpec;
}

/**
 * @brief 销毁typedef名称类型说明符
 */
static void destroyTypedefNameSpecifier(ASTNode* node) {
    if (!node) {
        return;
    }

    TypedefNameSpecifier* typeSpec = (TypedefNameSpecifier*)node;
    if (typeSpec->typedefName) {
        free(typeSpec->typedefName);
    }
    free(typeSpec);
}

// ==================== 辅助字符串转换函数 ====================

const char* basicTypeKindToString(BasicTypeKind kind) {
    switch (kind) {
        case BASIC_TYPE_VOID: return "void";
        case BASIC_TYPE_CHAR: return "char";
        case BASIC_TYPE_SHORT: return "short";
        case BASIC_TYPE_INT: return "int";
        case BASIC_TYPE_LONG: return "long";
        case BASIC_TYPE_FLOAT: return "float";
        case BASIC_TYPE_DOUBLE: return "double";
        case BASIC_TYPE_SIGNED: return "signed";
        case BASIC_TYPE_UNSIGNED: return "unsigned";
        case BASIC_TYPE_BOOL: return "_Bool";
        case BASIC_TYPE_COMPLEX: return "_Complex";
        default: return "?";
    }
}

const char* storageClassToString(StorageClassSpecifier storage) {
    switch (storage) {
        case STORAGE_CLASS_AUTO: return "auto";
        case STORAGE_CLASS_STATIC: return "static";
        case STORAGE_CLASS_EXTERN: return "extern";
        case STORAGE_CLASS_REGISTER: return "register";
        case STORAGE_CLASS_THREAD_LOCAL: return "_Thread_local";
        case STORAGE_CLASS_NONE: return "";
        default: return "?";
    }
}
