/**
 * @file ast.c
 * @brief AST基础实现
 *
 * 实现AST核心基础结构函数。
 */

#include "ast.h"
#include "ast_visitor.h"
#include "../../common/diagnostics/diagnostic_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================== 节点类型名称 ====================

const char* astNodeTypeName(ASTNodeType type) {
    switch (type) {
        // 基础类型
        case AST_NODE_EXPRESSION:
            return "Expression";
        case AST_NODE_STATEMENT:
            return "Statement";
        case AST_NODE_DECLARATION:
            return "Declaration";
        case AST_NODE_TYPE_SPECIFIER:
            return "TypeSpecifier";
        case AST_NODE_TRANSLATION_UNIT:
            return "TranslationUnit";

        // 表达式子类型
        case AST_NODE_LITERAL_EXPR:
            return "LiteralExpr";
        case AST_NODE_IDENTIFIER_EXPR:
            return "IdentifierExpr";
        case AST_NODE_BINARY_OPERATOR_EXPR:
            return "BinaryOperatorExpr";
        case AST_NODE_UNARY_OPERATOR_EXPR:
            return "UnaryOperatorExpr";
        case AST_NODE_ASSIGNMENT_EXPR:
            return "AssignmentExpr";
        case AST_NODE_TERNARY_EXPR:
            return "TernaryExpr";
        case AST_NODE_FUNCTION_CALL_EXPR:
            return "FunctionCallExpr";
        case AST_NODE_ARRAY_SUBSCRIPT_EXPR:
            return "ArraySubscriptExpr";
        case AST_NODE_MEMBER_ACCESS_EXPR:
            return "MemberAccessExpr";
        case AST_NODE_CAST_EXPR:
            return "CastExpr";

        // 语句子类型
        case AST_NODE_EXPRESSION_STATEMENT:
            return "ExpressionStatement";
        case AST_NODE_COMPOUND_STATEMENT:
            return "CompoundStatement";
        case AST_NODE_IF_STATEMENT:
            return "IfStatement";
        case AST_NODE_WHILE_STATEMENT:
            return "WhileStatement";
        case AST_NODE_DO_WHILE_STATEMENT:
            return "DoWhileStatement";
        case AST_NODE_FOR_STATEMENT:
            return "ForStatement";
        case AST_NODE_RETURN_STATEMENT:
            return "ReturnStatement";
        case AST_NODE_BREAK_STATEMENT:
            return "BreakStatement";
        case AST_NODE_CONTINUE_STATEMENT:
            return "ContinueStatement";
        case AST_NODE_SWITCH_STATEMENT:
            return "SwitchStatement";
        case AST_NODE_CASE_STATEMENT:
            return "CaseStatement";
        case AST_NODE_LABELED_STATEMENT:
            return "LabeledStatement";
        case AST_NODE_GOTO_STATEMENT:
            return "GotoStatement";

        // 声明子类型
        case AST_NODE_VARIABLE_DECLARATION:
            return "VariableDeclaration";
        case AST_NODE_FUNCTION_DECLARATION:
            return "FunctionDeclaration";
        case AST_NODE_STRUCT_DECLARATION:
            return "StructDeclaration";
        case AST_NODE_UNION_DECLARATION:
            return "UnionDeclaration";
        case AST_NODE_ENUM_DECLARATION:
            return "EnumDeclaration";
        case AST_NODE_TYPEDEF_DECLARATION:
            return "TypedefDeclaration";

        // 类型说明符子类型
        case AST_NODE_BASIC_TYPE_SPECIFIER:
            return "BasicTypeSpecifier";
        case AST_NODE_POINTER_TYPE_SPECIFIER:
            return "PointerTypeSpecifier";
        case AST_NODE_ARRAY_TYPE_SPECIFIER:
            return "ArrayTypeSpecifier";
        case AST_NODE_FUNCTION_TYPE_SPECIFIER:
            return "FunctionTypeSpecifier";
        case AST_NODE_STRUCT_TYPE_SPECIFIER:
            return "StructTypeSpecifier";
        case AST_NODE_UNION_TYPE_SPECIFIER:
            return "UnionTypeSpecifier";
        case AST_NODE_ENUM_TYPE_SPECIFIER:
            return "EnumTypeSpecifier";
        case AST_NODE_TYPEDEF_NAME_SPECIFIER:
            return "TypedefNameSpecifier";

        default:
            return "Unknown";
    }
}

// ==================== 基础AST节点函数 ====================

/**
 * @brief 默认的accept函数实现
 */
static void astNodeAcceptDefault(ASTNode* self, ASTVisitor* visitor) {
    if (!self || !visitor) {
        return;
    }

    // 默认实现：根据节点类型分派
    // 这个函数应该由各个具体的节点类型实现覆盖
    // 这里只是一个后备实现
}

/**
 * @brief 默认的destroy函数实现
 */
static void destroyASTNodeDefault(ASTNode* node) {
    if (!node) {
        return;
    }

    // 默认实现：只释放节点本身
    // 子类应该覆盖此函数以处理子节点的清理
    free(node);
}

ASTNode* createASTNode(ASTNodeType type, SourceLocation location) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    if (!node) {
        return NULL;
    }

    node->location = location;
    node->parent = NULL;
    node->nodeType = type;
    node->accept = astNodeAcceptDefault;
    node->destroy = destroyASTNodeDefault;

    return node;
}

void destroyASTNode(ASTNode* node) {
    if (!node) {
        return;
    }

    // 调用节点的虚函数
    if (node->destroy) {
        node->destroy(node);
    } else {
        free(node);
    }
}

// ==================== 翻译单元函数 ====================

/**
 * @brief 翻译单元的销毁函数
 */
static void destroyTranslationUnitNode(ASTNode* node) {
    if (!node) {
        return;
    }

    TranslationUnit* unit = (TranslationUnit*)node;

    // 销毁所有声明
    if (unit->declarations) {
        for (size_t i = 0; i < vectorSize(unit->declarations); i++) {
            ASTNode* decl = (ASTNode*)vectorGet(unit->declarations, i);
            if (decl) {
                DESTROY_AST_NODE(decl);
            }
        }
        vectorDestroy(unit->declarations, (void (*)(void*))destroyASTNode);
    }

    free(unit);
}

/**
 * @brief 翻译单元的accept函数
 */
static void translationUnitAccept(ASTNode* self, ASTVisitor* visitor) {
    if (!self || !visitor) {
        return;
    }

    if (visitor->visitTranslationUnit) {
        visitor->visitTranslationUnit(visitor, (TranslationUnit*)self);
    }
}

TranslationUnit* createTranslationUnit(void) {
    TranslationUnit* unit = (TranslationUnit*)calloc(1, sizeof(TranslationUnit));
    if (!unit) {
        return NULL;
    }

    unit->base.location = (SourceLocation){0};
    unit->base.parent = NULL;
    unit->base.nodeType = AST_NODE_TRANSLATION_UNIT;
    unit->base.accept = translationUnitAccept;
    unit->base.destroy = destroyTranslationUnitNode;

    unit->declarations = vectorCreate(sizeof(ASTNode*), 4);
    if (!unit->declarations) {
        free(unit);
        return NULL;
    }

    return unit;
}

void destroyTranslationUnit(TranslationUnit* unit) {
    if (!unit) {
        return;
    }

    if (unit->base.destroy) {
        unit->base.destroy((ASTNode*)unit);
    }
}

// ==================== 树遍历辅助函数 ====================

void astNodeSetParent(ASTNode* node, ASTNode* parent) {
    if (!node) {
        return;
    }

    node->parent = parent;
}

SourceLocation astNodeGetLocation(const ASTNode* node) {
    if (!node) {
        return (SourceLocation){0};
    }

    return node->location;
}
