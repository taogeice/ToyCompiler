/**
 * @file ast_dumper.c
 * @brief AST调试输出实现
 *
 * 提供AST树的文本可视化功能，用于调试和测试。
 */

#include "ast_dumper.h"
#include "ast_visitor.h"
#include "../../common/diagnostics/diagnostic_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================== ASTDumper结构 ====================

/**
 * @brief AST转储器
 *
 * 使用访问者模式实现AST树的遍历和输出。
 */
typedef struct {
    ASTVisitor base;              // 访问者基类
    FILE* output;                 // 输出流
    int indent;                   // 当前缩进级别
    int indentSize;               // 缩进大小（空格数）
    bool showLocation;            // 是否显示源位置
    bool showTypes;               // 是否显示类型信息
    bool colorOutput;             // 是否使用颜色输出
    Vector* nodeStack;            // 节点栈（用于追踪父节点）
    int nodeCount;                // 已访问节点计数
} ASTDumper;

// ==================== 内部辅助函数 ====================

/**
 * @brief 输出缩进
 */
static void printIndent(ASTDumper* dumper) {
    if (!dumper) return;
    for (int i = 0; i < dumper->indent * dumper->indentSize; i++) {
        fprintf(dumper->output, " ");
    }
}

/**
 * @brief 输出节点前缀
 */
static void printNodePrefix(ASTDumper* dumper, const char* nodeName, ASTNode* node) {
    printIndent(dumper);

    // 输出节点名称
    if (dumper->colorOutput) {
        fprintf(dumper->output, "\033[1;36m%s\033[0m", nodeName);
    } else {
        fprintf(dumper->output, "%s", nodeName);
    }

    // 输出源位置
    if (dumper->showLocation && node) {
        if (node->location.filename) {
            fprintf(dumper->output, " @ \033[0;33m%s:%d:%d\033[0m",
                    node->location.filename,
                    node->location.line,
                    node->location.column);
        } else {
            fprintf(dumper->output, " @ \033[0;33m%d:%d\033[0m",
                    node->location.line,
                    node->location.column);
        }
    }

    dumper->nodeCount++;
}

/**
 * @brief 输出类型信息
 */
static void printTypeInfo(ASTDumper* dumper, Expression* expr) {
    if (!dumper->showTypes || !expr || !expr->type) {
        return;
    }

    fprintf(dumper->output, " \033[0;32m[type:]\033[0m");
    // TODO: 输出类型名称
}

// ==================== 翻译单元访问函数 ====================

static void visitTranslationUnit(ASTVisitor* visitor, TranslationUnit* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "TranslationUnit", (ASTNode*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->declarations) {
        for (size_t i = 0; i < vectorSize(node->declarations); i++) {
            ASTNode* child = (ASTNode*)vectorGet(node->declarations, i);
            astNodeAccept(child, visitor);
        }
    }
    dumper->indent--;
}

// ==================== 表达式访问函数 ====================

static void visitLiteralExpr(ASTVisitor* visitor, LiteralExpr* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "LiteralExpr", (ASTNode*)node);

    if (node->literalToken->lexeme) {
        fprintf(dumper->output, ": \033[1;33m'%s'\033[0m", node->literalToken->lexeme);
    }

    printTypeInfo(dumper, (Expression*)node);
    fprintf(dumper->output, "\n");
}

static void visitIdentifierExpr(ASTVisitor* visitor, IdentifierExpr* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "IdentifierExpr", (ASTNode*)node);

    if (node->name) {
        fprintf(dumper->output, ": \033[1;33m'%s'\033[0m", node->name);
    }

    printTypeInfo(dumper, (Expression*)node);
    fprintf(dumper->output, "\n");
}

static void visitBinaryOperatorExpr(ASTVisitor* visitor, BinaryOperatorExpr* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "BinaryOperatorExpr", (ASTNode*)node);

    const char* opStr = binaryOperatorToString(node->op);
    fprintf(dumper->output, ": \033[1;35m%s\033[0m", opStr);
    printTypeInfo(dumper, (Expression*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->left) {
        astNodeAccept((ASTNode*)node->left, visitor);
    }
    if (node->right) {
        astNodeAccept((ASTNode*)node->right, visitor);
    }
    dumper->indent--;
}

static void visitUnaryOperatorExpr(ASTVisitor* visitor, UnaryOperatorExpr* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "UnaryOperatorExpr", (ASTNode*)node);

    const char* opStr = unaryOperatorToString(node->op);
    fprintf(dumper->output, ": \033[1;35m%s\033[0m", opStr);

    if (node->isPrefix) {
        fprintf(dumper->output, " (prefix)");
    } else {
        fprintf(dumper->output, " (postfix)");
    }

    printTypeInfo(dumper, (Expression*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->operand) {
        astNodeAccept((ASTNode*)node->operand, visitor);
    }
    dumper->indent--;
}

static void visitAssignmentExpr(ASTVisitor* visitor, AssignmentExpr* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "AssignmentExpr", (ASTNode*)node);

    const char* opStr = assignmentKindToString(node->kind);
    fprintf(dumper->output, ": \033[1;35m%s\033[0m", opStr);
    printTypeInfo(dumper, (Expression*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->left) {
        astNodeAccept((ASTNode*)node->left, visitor);
    }
    if (node->right) {
        astNodeAccept((ASTNode*)node->right, visitor);
    }
    dumper->indent--;
}

static void visitTernaryExpr(ASTVisitor* visitor, TernaryExpr* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "TernaryExpr", (ASTNode*)node);
    printTypeInfo(dumper, (Expression*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->condition) {
        astNodeAccept((ASTNode*)node->condition, visitor);
    }
    if (node->thenExpr) {
        astNodeAccept((ASTNode*)node->thenExpr, visitor);
    }
    if (node->elseExpr) {
        astNodeAccept((ASTNode*)node->elseExpr, visitor);
    }
    dumper->indent--;
}

static void visitFunctionCallExpr(ASTVisitor* visitor, FunctionCallExpr* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "FunctionCallExpr", (ASTNode*)node);
    printTypeInfo(dumper, (Expression*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->callee) {
        astNodeAccept((ASTNode*)node->callee, visitor);
    }
    if (node->arguments && vectorSize(node->arguments) > 0) {
        printIndent(dumper);
        fprintf(dumper->output, "Arguments (%zu):\n", vectorSize(node->arguments));
        dumper->indent++;
        for (size_t i = 0; i < vectorSize(node->arguments); i++) {
            Expression* arg = (Expression*)vectorGet(node->arguments, i);
            astNodeAccept((ASTNode*)arg, visitor);
        }
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitArraySubscriptExpr(ASTVisitor* visitor, ArraySubscriptExpr* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "ArraySubscriptExpr", (ASTNode*)node);
    printTypeInfo(dumper, (Expression*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->array) {
        astNodeAccept((ASTNode*)node->array, visitor);
    }
    if (node->index) {
        astNodeAccept((ASTNode*)node->index, visitor);
    }
    dumper->indent--;
}

static void visitMemberAccessExpr(ASTVisitor* visitor, MemberAccessExpr* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "MemberAccessExpr", (ASTNode*)node);

    if (node->memberName) {
        fprintf(dumper->output, ": \033[1;33m'%s'\033[0m", node->memberName);
    }

    fprintf(dumper->output, " (\033[1;35m%s\033[0m)", node->isArrow ? "->" : ".");
    printTypeInfo(dumper, (Expression*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->baseExpr) {
        astNodeAccept((ASTNode*)node->baseExpr, visitor);
    }
    dumper->indent--;
}

static void visitCastExpr(ASTVisitor* visitor, CastExpr* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "CastExpr", (ASTNode*)node);
    printTypeInfo(dumper, (Expression*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->targetType) {
        printIndent(dumper);
        fprintf(dumper->output, "TargetType:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->targetType, visitor);
        dumper->indent--;
    }
    if (node->operand) {
        astNodeAccept((ASTNode*)node->operand, visitor);
    }
    dumper->indent--;
}

// ==================== 语句访问函数 ====================

static void visitExpressionStatement(ASTVisitor* visitor, ExpressionStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "ExpressionStatement", (ASTNode*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->expression) {
        astNodeAccept((ASTNode*)node->expression, visitor);
    }
    dumper->indent--;
}

static void visitCompoundStatement(ASTVisitor* visitor, CompoundStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "CompoundStatement", (ASTNode*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;

    // 输出局部声明
    if (node->declarations && vectorSize(node->declarations) > 0) {
        printIndent(dumper);
        fprintf(dumper->output, "Declarations (%zu):\n", vectorSize(node->declarations));
        dumper->indent++;
        for (size_t i = 0; i < vectorSize(node->declarations); i++) {
            ASTNode* decl = (ASTNode*)vectorGet(node->declarations, i);
            astNodeAccept(decl, visitor);
        }
        dumper->indent--;
    }

    // 输出语句
    if (node->statements && vectorSize(node->statements) > 0) {
        printIndent(dumper);
        fprintf(dumper->output, "Statements (%zu):\n", vectorSize(node->statements));
        dumper->indent++;
        for (size_t i = 0; i < vectorSize(node->statements); i++) {
            ASTNode* stmt = (ASTNode*)vectorGet(node->statements, i);
            astNodeAccept(stmt, visitor);
        }
        dumper->indent--;
    }

    dumper->indent--;
}

static void visitIfStatement(ASTVisitor* visitor, IfStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "IfStatement", (ASTNode*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->condition) {
        printIndent(dumper);
        fprintf(dumper->output, "Condition:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->condition, visitor);
        dumper->indent--;
    }
    if (node->thenStmt) {
        printIndent(dumper);
        fprintf(dumper->output, "Then:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->thenStmt, visitor);
        dumper->indent--;
    }
    if (node->elseStmt) {
        printIndent(dumper);
        fprintf(dumper->output, "Else:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->elseStmt, visitor);
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitWhileStatement(ASTVisitor* visitor, WhileStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "WhileStatement", (ASTNode*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->condition) {
        printIndent(dumper);
        fprintf(dumper->output, "Condition:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->condition, visitor);
        dumper->indent--;
    }
    if (node->body) {
        printIndent(dumper);
        fprintf(dumper->output, "Body:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->body, visitor);
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitDoWhileStatement(ASTVisitor* visitor, DoWhileStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "DoWhileStatement", (ASTNode*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->body) {
        printIndent(dumper);
        fprintf(dumper->output, "Body:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->body, visitor);
        dumper->indent--;
    }
    if (node->condition) {
        printIndent(dumper);
        fprintf(dumper->output, "Condition:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->condition, visitor);
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitForStatement(ASTVisitor* visitor, ForStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "ForStatement", (ASTNode*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->init) {
        printIndent(dumper);
        fprintf(dumper->output, "Init:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->init, visitor);
        dumper->indent--;
    }
    if (node->condition) {
        printIndent(dumper);
        fprintf(dumper->output, "Condition:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->condition, visitor);
        dumper->indent--;
    }
    if (node->increment) {
        printIndent(dumper);
        fprintf(dumper->output, "Increment:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->increment, visitor);
        dumper->indent--;
    }
    if (node->body) {
        printIndent(dumper);
        fprintf(dumper->output, "Body:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->body, visitor);
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitReturnStatement(ASTVisitor* visitor, ReturnStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "ReturnStatement", (ASTNode*)node);
    fprintf(dumper->output, "\n");

    if (node->returnValue) {
        dumper->indent++;
        astNodeAccept((ASTNode*)node->returnValue, visitor);
        dumper->indent--;
    }
}

static void visitBreakStatement(ASTVisitor* visitor, BreakStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "BreakStatement", (ASTNode*)node);
    fprintf(dumper->output, "\n");
}

static void visitContinueStatement(ASTVisitor* visitor, ContinueStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "ContinueStatement", (ASTNode*)node);
    fprintf(dumper->output, "\n");
}

static void visitSwitchStatement(ASTVisitor* visitor, SwitchStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "SwitchStatement", (ASTNode*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->condition) {
        printIndent(dumper);
        fprintf(dumper->output, "Condition:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->condition, visitor);
        dumper->indent--;
    }
    if (node->cases && vectorSize(node->cases) > 0) {
        printIndent(dumper);
        fprintf(dumper->output, "Cases (%zu):\n", vectorSize(node->cases));
        dumper->indent++;
        for (size_t i = 0; i < vectorSize(node->cases); i++) {
            ASTNode* caseStmt = (ASTNode*)vectorGet(node->cases, i);
            astNodeAccept(caseStmt, visitor);
        }
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitCaseStatement(ASTVisitor* visitor, CaseStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;

    if (node->kind == CASE_LABEL) {
        printNodePrefix(dumper, "Case", (ASTNode*)node);
        if (node->value) {
            fprintf(dumper->output, ": ");
            astNodeAccept((ASTNode*)node->value, visitor);
        }
    } else {
        printNodePrefix(dumper, "Default", (ASTNode*)node);
    }
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->statement) {
        astNodeAccept((ASTNode*)node->statement, visitor);
    }
    dumper->indent--;
}

static void visitLabeledStatement(ASTVisitor* visitor, LabeledStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "LabeledStatement", (ASTNode*)node);

    if (node->labelName) {
        fprintf(dumper->output, ": \033[1;33m'%s'\033[0m", node->labelName);
    }
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->statement) {
        astNodeAccept((ASTNode*)node->statement, visitor);
    }
    dumper->indent--;
}

static void visitGotoStatement(ASTVisitor* visitor, GotoStatement* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "GotoStatement", (ASTNode*)node);

    if (node->labelName) {
        fprintf(dumper->output, ": \033[1;33m'%s'\033[0m", node->labelName);
    }
    fprintf(dumper->output, "\n");
}

// ==================== 声明访问函数 ====================

static void visitVariableDeclaration(ASTVisitor* visitor, VariableDeclaration* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "VariableDeclaration", (ASTNode*)node);

    if (node->base.name) {
        fprintf(dumper->output, ": \033[1;32m'%s'\033[0m", node->base.name);
    }

    if (node->isConst) {
        fprintf(dumper->output, " \033[1;34m[const]\033[0m");
    }
    if (node->isVolatile) {
        fprintf(dumper->output, " \033[1;34m[volatile]\033[0m");
    }

    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->type) {
        printIndent(dumper);
        fprintf(dumper->output, "Type:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->type, visitor);
        dumper->indent--;
    }
    if (node->initializer) {
        printIndent(dumper);
        fprintf(dumper->output, "Initializer:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->initializer, visitor);
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitFunctionDeclaration(ASTVisitor* visitor, FunctionDeclaration* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "FunctionDeclaration", (ASTNode*)node);

    if (node->base.name) {
        fprintf(dumper->output, ": \033[1;32m'%s'\033[0m", node->base.name);
    }

    if (node->isInline) {
        fprintf(dumper->output, " \033[1;34m[inline]\033[0m");
    }
    if (node->isNoreturn) {
        fprintf(dumper->output, " \033[1;34m[noreturn]\033[0m");
    }

    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->returnType) {
        printIndent(dumper);
        fprintf(dumper->output, "ReturnType:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->returnType, visitor);
        dumper->indent--;
    }
    if (node->parameters && vectorSize(node->parameters) > 0) {
        printIndent(dumper);
        fprintf(dumper->output, "Parameters (%zu):\n", vectorSize(node->parameters));
        dumper->indent++;
        for (size_t i = 0; i < vectorSize(node->parameters); i++) {
            ASTNode* param = (ASTNode*)vectorGet(node->parameters, i);
            astNodeAccept(param, visitor);
        }
        dumper->indent--;
    }
    if (node->body) {
        printIndent(dumper);
        fprintf(dumper->output, "Body:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->body, visitor);
        dumper->indent--;
    } else {
        printIndent(dumper);
        fprintf(dumper->output, "(\033[1;33mprototype\033[0m)\n");
    }
    dumper->indent--;
}

static void visitStructDeclaration(ASTVisitor* visitor, StructDeclaration* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "StructDeclaration", (ASTNode*)node);

    if (node->base.name) {
        fprintf(dumper->output, ": \033[1;32m'%s'\033[0m", node->base.name);
    } else {
        fprintf(dumper->output, ": \033[1;33m<anonymous>\033[0m");
    }

    if (node->isPacked) {
        fprintf(dumper->output, " \033[1;34m[packed]\033[0m");
    }

    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->members && vectorSize(node->members) > 0) {
        printIndent(dumper);
        fprintf(dumper->output, "Members (%zu):\n", vectorSize(node->members));
        dumper->indent++;
        for (size_t i = 0; i < vectorSize(node->members); i++) {
            ASTNode* member = (ASTNode*)vectorGet(node->members, i);
            astNodeAccept(member, visitor);
        }
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitUnionDeclaration(ASTVisitor* visitor, UnionDeclaration* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "UnionDeclaration", (ASTNode*)node);

    if (node->base.name) {
        fprintf(dumper->output, ": \033[1;32m'%s'\033[0m", node->base.name);
    } else {
        fprintf(dumper->output, ": \033[1;33m<anonymous>\033[0m");
    }

    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->members && vectorSize(node->members) > 0) {
        printIndent(dumper);
        fprintf(dumper->output, "Members (%zu):\n", vectorSize(node->members));
        dumper->indent++;
        for (size_t i = 0; i < vectorSize(node->members); i++) {
            ASTNode* member = (ASTNode*)vectorGet(node->members, i);
            astNodeAccept(member, visitor);
        }
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitEnumDeclaration(ASTVisitor* visitor, EnumDeclaration* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "EnumDeclaration", (ASTNode*)node);

    if (node->base.name) {
        fprintf(dumper->output, ": \033[1;32m'%s'\033[0m", node->base.name);
    } else {
        fprintf(dumper->output, ": \033[1;33m<anonymous>\033[0m");
    }

    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->constants && vectorSize(node->constants) > 0) {
        printIndent(dumper);
        fprintf(dumper->output, "Constants (%zu):\n", vectorSize(node->constants));
        dumper->indent++;
        for (size_t i = 0; i < vectorSize(node->constants); i++) {
            EnumConstant* constant = (EnumConstant*)vectorGet(node->constants, i);
            printIndent(dumper);
            fprintf(dumper->output, "\033[1;32m%s\033[0m", constant->name);
            if (constant->value) {
                fprintf(dumper->output, " = ");
                // TODO: 输出值
            }
            fprintf(dumper->output, "\n");
        }
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitTypedefDeclaration(ASTVisitor* visitor, TypedefDeclaration* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "TypedefDeclaration", (ASTNode*)node);

    if (node->base.name) {
        fprintf(dumper->output, ": \033[1;32m'%s'\033[0m", node->base.name);
    }

    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->aliasedType) {
        printIndent(dumper);
        fprintf(dumper->output, "AliasedType:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->aliasedType, visitor);
        dumper->indent--;
    }
    dumper->indent--;
}

// ==================== 类型说明符访问函数 ====================

static void visitBasicTypeSpecifier(ASTVisitor* visitor, BasicTypeSpecifier* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "BasicTypeSpecifier", (ASTNode*)node);

    const char* typeStr = basicTypeKindToString(node->kind);
    fprintf(dumper->output, ": \033[1;35m%s\033[0m", typeStr);

    if (node->isLong) {
        fprintf(dumper->output, " \033[1;34m[long]\033[0m");
    }
    if (node->isShort) {
        fprintf(dumper->output, " \033[1;34m[short]\033[0m");
    }
    if (node->isSigned) {
        fprintf(dumper->output, " \033[1;34m[signed]\033[0m");
    }
    if (node->isUnsigned) {
        fprintf(dumper->output, " \033[1;34m[unsigned]\033[0m");
    }

    fprintf(dumper->output, "\n");
}

static void visitPointerTypeSpecifier(ASTVisitor* visitor, PointerTypeSpecifier* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "PointerTypeSpecifier", (ASTNode*)node);
    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->baseType) {
        astNodeAccept((ASTNode*)node->baseType, visitor);
    }
    dumper->indent--;
}

static void visitArrayTypeSpecifier(ASTVisitor* visitor, ArrayTypeSpecifier* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "ArrayTypeSpecifier", (ASTNode*)node);

    if (node->isVariableLength) {
        fprintf(dumper->output, " \033[1;34m[VLA]\033[0m");
    }

    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->elementType) {
        printIndent(dumper);
        fprintf(dumper->output, "ElementType:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->elementType, visitor);
        dumper->indent--;
    }
    if (node->size) {
        printIndent(dumper);
        fprintf(dumper->output, "Size:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->size, visitor);
        dumper->indent--;
    } else {
        printIndent(dumper);
        fprintf(dumper->output, "Size: \033[1;33m<unspecified>\033[0m\n");
    }
    dumper->indent--;
}

static void visitFunctionTypeSpecifier(ASTVisitor* visitor, FunctionTypeSpecifier* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "FunctionTypeSpecifier", (ASTNode*)node);

    if (node->isVariadic) {
        fprintf(dumper->output, " \033[1;34m[variadic]\033[0m");
    }

    fprintf(dumper->output, "\n");

    dumper->indent++;
    if (node->returnType) {
        printIndent(dumper);
        fprintf(dumper->output, "ReturnType:\n");
        dumper->indent++;
        astNodeAccept((ASTNode*)node->returnType, visitor);
        dumper->indent--;
    }
    if (node->parameterTypes && vectorSize(node->parameterTypes) > 0) {
        printIndent(dumper);
        fprintf(dumper->output, "ParameterTypes (%zu):\n", vectorSize(node->parameterTypes));
        dumper->indent++;
        for (size_t i = 0; i < vectorSize(node->parameterTypes); i++) {
            ASTNode* paramType = (ASTNode*)vectorGet(node->parameterTypes, i);
            astNodeAccept(paramType, visitor);
        }
        dumper->indent--;
    }
    dumper->indent--;
}

static void visitStructTypeSpecifier(ASTVisitor* visitor, StructTypeSpecifier* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "StructTypeSpecifier", (ASTNode*)node);

    if (node->name) {
        fprintf(dumper->output, ": \033[1;32m'%s'\033[0m", node->name);
    } else {
        fprintf(dumper->output, ": \033[1;33m<anonymous>\033[0m");
    }

    if (!node->declaration) {
        fprintf(dumper->output, " \033[1;33m[forward]\033[0m");
    }

    fprintf(dumper->output, "\n");
}

static void visitUnionTypeSpecifier(ASTVisitor* visitor, UnionTypeSpecifier* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "UnionTypeSpecifier", (ASTNode*)node);

    if (node->name) {
        fprintf(dumper->output, ": \033[1;32m'%s'\033[0m", node->name);
    } else {
        fprintf(dumper->output, ": \033[1;33m<anonymous>\033[0m");
    }

    if (!node->declaration) {
        fprintf(dumper->output, " \033[1;33m[forward]\033[0m");
    }

    fprintf(dumper->output, "\n");
}

static void visitEnumTypeSpecifier(ASTVisitor* visitor, EnumTypeSpecifier* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "EnumTypeSpecifier", (ASTNode*)node);

    if (node->name) {
        fprintf(dumper->output, ": \033[1;32m'%s'\033[0m", node->name);
    } else {
        fprintf(dumper->output, ": \033[1;33m<anonymous>\033[0m");
    }

    if (!node->declaration) {
        fprintf(dumper->output, " \033[1;33m[forward]\033[0m");
    }

    fprintf(dumper->output, "\n");
}

static void visitTypedefNameSpecifier(ASTVisitor* visitor, TypedefNameSpecifier* node) {
    ASTDumper* dumper = (ASTDumper*)visitor;
    printNodePrefix(dumper, "TypedefNameSpecifier", (ASTNode*)node);

    if (node->typedefName) {
        fprintf(dumper->output, ": \033[1;32m'%s'\033[0m", node->typedefName);
    }

    fprintf(dumper->output, "\n");
}

// ==================== 公共API函数 ====================

ASTDumperHandle* createASTDumper(FILE* output) {
    if (!output) {
        output = stdout;
    }

    ASTDumper* dumper = (ASTDumper*)calloc(1, sizeof(ASTDumper));
    if (!dumper) {
        return NULL;
    }

    // 初始化访问者函数
    dumper->base.visitTranslationUnit = visitTranslationUnit;
    dumper->base.visitLiteralExpr = visitLiteralExpr;
    dumper->base.visitIdentifierExpr = visitIdentifierExpr;
    dumper->base.visitBinaryOperatorExpr = visitBinaryOperatorExpr;
    dumper->base.visitUnaryOperatorExpr = visitUnaryOperatorExpr;
    dumper->base.visitAssignmentExpr = visitAssignmentExpr;
    dumper->base.visitTernaryExpr = visitTernaryExpr;
    dumper->base.visitFunctionCallExpr = visitFunctionCallExpr;
    dumper->base.visitArraySubscriptExpr = visitArraySubscriptExpr;
    dumper->base.visitMemberAccessExpr = visitMemberAccessExpr;
    dumper->base.visitCastExpr = visitCastExpr;

    dumper->base.visitExpressionStatement = visitExpressionStatement;
    dumper->base.visitCompoundStatement = visitCompoundStatement;
    dumper->base.visitIfStatement = visitIfStatement;
    dumper->base.visitWhileStatement = visitWhileStatement;
    dumper->base.visitDoWhileStatement = visitDoWhileStatement;
    dumper->base.visitForStatement = visitForStatement;
    dumper->base.visitReturnStatement = visitReturnStatement;
    dumper->base.visitBreakStatement = visitBreakStatement;
    dumper->base.visitContinueStatement = visitContinueStatement;
    dumper->base.visitSwitchStatement = visitSwitchStatement;
    dumper->base.visitCaseStatement = visitCaseStatement;
    dumper->base.visitLabeledStatement = visitLabeledStatement;
    dumper->base.visitGotoStatement = visitGotoStatement;

    dumper->base.visitVariableDeclaration = visitVariableDeclaration;
    dumper->base.visitFunctionDeclaration = visitFunctionDeclaration;
    dumper->base.visitStructDeclaration = visitStructDeclaration;
    dumper->base.visitUnionDeclaration = visitUnionDeclaration;
    dumper->base.visitEnumDeclaration = visitEnumDeclaration;
    dumper->base.visitTypedefDeclaration = visitTypedefDeclaration;

    dumper->base.visitBasicTypeSpecifier = visitBasicTypeSpecifier;
    dumper->base.visitPointerTypeSpecifier = visitPointerTypeSpecifier;
    dumper->base.visitArrayTypeSpecifier = visitArrayTypeSpecifier;
    dumper->base.visitFunctionTypeSpecifier = visitFunctionTypeSpecifier;
    dumper->base.visitStructTypeSpecifier = visitStructTypeSpecifier;
    dumper->base.visitUnionTypeSpecifier = visitUnionTypeSpecifier;
    dumper->base.visitEnumTypeSpecifier = visitEnumTypeSpecifier;
    dumper->base.visitTypedefNameSpecifier = visitTypedefNameSpecifier;

    // 设置默认值
    dumper->output = output;
    dumper->indent = 0;
    dumper->indentSize = 2;
    dumper->showLocation = false;
    dumper->showTypes = false;
    dumper->colorOutput = true;
    dumper->nodeCount = 0;

    return (ASTDumperHandle*)dumper;
}

void destroyASTDumper(ASTDumperHandle* handle) {
    if (!handle) {
        return;
    }

    ASTDumper* dumper = (ASTDumper*)handle;

    if (dumper->nodeStack) {
        vectorDestroy(dumper->nodeStack, NULL);
    }

    free(dumper);
}

void astDumperSetIndent(ASTDumperHandle* handle, int indentSize) {
    if (!handle) {
        return;
    }

    ASTDumper* dumper = (ASTDumper*)handle;
    dumper->indentSize = indentSize;
}

void astDumperSetShowLocation(ASTDumperHandle* handle, bool show) {
    if (!handle) {
        return;
    }

    ASTDumper* dumper = (ASTDumper*)handle;
    dumper->showLocation = show;
}

void astDumperSetShowTypes(ASTDumperHandle* handle, bool show) {
    if (!handle) {
        return;
    }

    ASTDumper* dumper = (ASTDumper*)handle;
    dumper->showTypes = show;
}

void astDumperSetColorOutput(ASTDumperHandle* handle, bool color) {
    if (!handle) {
        return;
    }

    ASTDumper* dumper = (ASTDumper*)handle;
    dumper->colorOutput = color;
}

void astDumperDump(ASTDumperHandle* handle, ASTNode* root) {
    if (!handle || !root) {
        return;
    }

    ASTDumper* dumper = (ASTDumper*)handle;
    dumper->nodeCount = 0;

    fprintf(dumper->output, "\033[1;37m=== AST Dump ===\033[0m\n");
    astNodeAccept(root, (ASTVisitor*)dumper);
    fprintf(dumper->output, "\033[1;37m=== Total: %d nodes ===\033[0m\n", dumper->nodeCount);
}

int astDumperGetNodeCount(ASTDumperHandle* handle) {
    if (!handle) {
        return 0;
    }

    ASTDumper* dumper = (ASTDumper*)handle;
    return dumper->nodeCount;
}
