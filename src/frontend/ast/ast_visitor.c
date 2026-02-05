/**
 * @file ast_visitor.c
 * @brief AST访问者模式实现
 *
 * 实现访问者模式，支持AST树遍历和操作。
 */

#include "ast_visitor.h"
#include "../../common/diagnostics/diagnostic_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================== 内部辅助函数 ====================

/**
 * @brief 获取节点类型对应的访问函数名称
 */
static const char* getVisitorFunctionName(ASTNodeType nodeType) {
    switch (nodeType) {
        case AST_NODE_TRANSLATION_UNIT:
            return "visitTranslationUnit";

        // 表达式节点
        case AST_NODE_LITERAL_EXPR:
            return "visitLiteralExpr";
        case AST_NODE_IDENTIFIER_EXPR:
            return "visitIdentifierExpr";
        case AST_NODE_BINARY_OPERATOR_EXPR:
            return "visitBinaryOperatorExpr";
        case AST_NODE_UNARY_OPERATOR_EXPR:
            return "visitUnaryOperatorExpr";
        case AST_NODE_ASSIGNMENT_EXPR:
            return "visitAssignmentExpr";
        case AST_NODE_TERNARY_EXPR:
            return "visitTernaryExpr";
        case AST_NODE_FUNCTION_CALL_EXPR:
            return "visitFunctionCallExpr";
        case AST_NODE_ARRAY_SUBSCRIPT_EXPR:
            return "visitArraySubscriptExpr";
        case AST_NODE_MEMBER_ACCESS_EXPR:
            return "visitMemberAccessExpr";
        case AST_NODE_CAST_EXPR:
            return "visitCastExpr";

        // 语句节点
        case AST_NODE_EXPRESSION_STATEMENT:
            return "visitExpressionStatement";
        case AST_NODE_COMPOUND_STATEMENT:
            return "visitCompoundStatement";
        case AST_NODE_IF_STATEMENT:
            return "visitIfStatement";
        case AST_NODE_WHILE_STATEMENT:
            return "visitWhileStatement";
        case AST_NODE_DO_WHILE_STATEMENT:
            return "visitDoWhileStatement";
        case AST_NODE_FOR_STATEMENT:
            return "visitForStatement";
        case AST_NODE_RETURN_STATEMENT:
            return "visitReturnStatement";
        case AST_NODE_BREAK_STATEMENT:
            return "visitBreakStatement";
        case AST_NODE_CONTINUE_STATEMENT:
            return "visitContinueStatement";
        case AST_NODE_SWITCH_STATEMENT:
            return "visitSwitchStatement";
        case AST_NODE_CASE_STATEMENT:
            return "visitCaseStatement";
        case AST_NODE_LABELED_STATEMENT:
            return "visitLabeledStatement";
        case AST_NODE_GOTO_STATEMENT:
            return "visitGotoStatement";

        // 声明节点
        case AST_NODE_VARIABLE_DECLARATION:
            return "visitVariableDeclaration";
        case AST_NODE_FUNCTION_DECLARATION:
            return "visitFunctionDeclaration";
        case AST_NODE_STRUCT_DECLARATION:
            return "visitStructDeclaration";
        case AST_NODE_UNION_DECLARATION:
            return "visitUnionDeclaration";
        case AST_NODE_ENUM_DECLARATION:
            return "visitEnumDeclaration";
        case AST_NODE_TYPEDEF_DECLARATION:
            return "visitTypedefDeclaration";

        // 类型说明符节点
        case AST_NODE_BASIC_TYPE_SPECIFIER:
            return "visitBasicTypeSpecifier";
        case AST_NODE_POINTER_TYPE_SPECIFIER:
            return "visitPointerTypeSpecifier";
        case AST_NODE_ARRAY_TYPE_SPECIFIER:
            return "visitArrayTypeSpecifier";
        case AST_NODE_FUNCTION_TYPE_SPECIFIER:
            return "visitFunctionTypeSpecifier";
        case AST_NODE_STRUCT_TYPE_SPECIFIER:
            return "visitStructTypeSpecifier";
        case AST_NODE_UNION_TYPE_SPECIFIER:
            return "visitUnionTypeSpecifier";
        case AST_NODE_ENUM_TYPE_SPECIFIER:
            return "visitEnumTypeSpecifier";
        case AST_NODE_TYPEDEF_NAME_SPECIFIER:
            return "visitTypedefNameSpecifier";

        default:
            return "visitUnknown";
    }
}

// ==================== AST节点接受函数实现 ====================

void astNodeAccept(ASTNode* node, ASTVisitor* visitor) {
    if (!node || !visitor) {
        return;
    }

    // 调用访问前钩子
    if (visitor->beforeVisit && !visitor->beforeVisit(visitor, node)) {
        // 钩子返回false，跳过此节点
        return;
    }

    // 根据节点类型调用相应的访问函数
    switch (node->nodeType) {
        case AST_NODE_TRANSLATION_UNIT:
            if (visitor->visitTranslationUnit) {
                visitor->visitTranslationUnit(visitor, (TranslationUnit*)node);
            }
            break;

        // 表达式节点
        case AST_NODE_LITERAL_EXPR:
            if (visitor->visitLiteralExpr) {
                visitor->visitLiteralExpr(visitor, (LiteralExpr*)node);
            }
            break;

        case AST_NODE_IDENTIFIER_EXPR:
            if (visitor->visitIdentifierExpr) {
                visitor->visitIdentifierExpr(visitor, (IdentifierExpr*)node);
            }
            break;

        case AST_NODE_BINARY_OPERATOR_EXPR:
            if (visitor->visitBinaryOperatorExpr) {
                visitor->visitBinaryOperatorExpr(visitor, (BinaryOperatorExpr*)node);
            }
            break;

        case AST_NODE_UNARY_OPERATOR_EXPR:
            if (visitor->visitUnaryOperatorExpr) {
                visitor->visitUnaryOperatorExpr(visitor, (UnaryOperatorExpr*)node);
            }
            break;

        case AST_NODE_ASSIGNMENT_EXPR:
            if (visitor->visitAssignmentExpr) {
                visitor->visitAssignmentExpr(visitor, (AssignmentExpr*)node);
            }
            break;

        case AST_NODE_TERNARY_EXPR:
            if (visitor->visitTernaryExpr) {
                visitor->visitTernaryExpr(visitor, (TernaryExpr*)node);
            }
            break;

        case AST_NODE_FUNCTION_CALL_EXPR:
            if (visitor->visitFunctionCallExpr) {
                visitor->visitFunctionCallExpr(visitor, (FunctionCallExpr*)node);
            }
            break;

        case AST_NODE_ARRAY_SUBSCRIPT_EXPR:
            if (visitor->visitArraySubscriptExpr) {
                visitor->visitArraySubscriptExpr(visitor, (ArraySubscriptExpr*)node);
            }
            break;

        case AST_NODE_MEMBER_ACCESS_EXPR:
            if (visitor->visitMemberAccessExpr) {
                visitor->visitMemberAccessExpr(visitor, (MemberAccessExpr*)node);
            }
            break;

        case AST_NODE_CAST_EXPR:
            if (visitor->visitCastExpr) {
                visitor->visitCastExpr(visitor, (CastExpr*)node);
            }
            break;

        // 语句节点
        case AST_NODE_EXPRESSION_STATEMENT:
            if (visitor->visitExpressionStatement) {
                visitor->visitExpressionStatement(visitor, (ExpressionStatement*)node);
            }
            break;

        case AST_NODE_COMPOUND_STATEMENT:
            if (visitor->visitCompoundStatement) {
                visitor->visitCompoundStatement(visitor, (CompoundStatement*)node);
            }
            break;

        case AST_NODE_IF_STATEMENT:
            if (visitor->visitIfStatement) {
                visitor->visitIfStatement(visitor, (IfStatement*)node);
            }
            break;

        case AST_NODE_WHILE_STATEMENT:
            if (visitor->visitWhileStatement) {
                visitor->visitWhileStatement(visitor, (WhileStatement*)node);
            }
            break;

        case AST_NODE_DO_WHILE_STATEMENT:
            if (visitor->visitDoWhileStatement) {
                visitor->visitDoWhileStatement(visitor, (DoWhileStatement*)node);
            }
            break;

        case AST_NODE_FOR_STATEMENT:
            if (visitor->visitForStatement) {
                visitor->visitForStatement(visitor, (ForStatement*)node);
            }
            break;

        case AST_NODE_RETURN_STATEMENT:
            if (visitor->visitReturnStatement) {
                visitor->visitReturnStatement(visitor, (ReturnStatement*)node);
            }
            break;

        case AST_NODE_BREAK_STATEMENT:
            if (visitor->visitBreakStatement) {
                visitor->visitBreakStatement(visitor, (BreakStatement*)node);
            }
            break;

        case AST_NODE_CONTINUE_STATEMENT:
            if (visitor->visitContinueStatement) {
                visitor->visitContinueStatement(visitor, (ContinueStatement*)node);
            }
            break;

        case AST_NODE_SWITCH_STATEMENT:
            if (visitor->visitSwitchStatement) {
                visitor->visitSwitchStatement(visitor, (SwitchStatement*)node);
            }
            break;

        case AST_NODE_CASE_STATEMENT:
            if (visitor->visitCaseStatement) {
                visitor->visitCaseStatement(visitor, (CaseStatement*)node);
            }
            break;

        case AST_NODE_LABELED_STATEMENT:
            if (visitor->visitLabeledStatement) {
                visitor->visitLabeledStatement(visitor, (LabeledStatement*)node);
            }
            break;

        case AST_NODE_GOTO_STATEMENT:
            if (visitor->visitGotoStatement) {
                visitor->visitGotoStatement(visitor, (GotoStatement*)node);
            }
            break;

        // 声明节点
        case AST_NODE_VARIABLE_DECLARATION:
            if (visitor->visitVariableDeclaration) {
                visitor->visitVariableDeclaration(visitor, (VariableDeclaration*)node);
            }
            break;

        case AST_NODE_FUNCTION_DECLARATION:
            if (visitor->visitFunctionDeclaration) {
                visitor->visitFunctionDeclaration(visitor, (FunctionDeclaration*)node);
            }
            break;

        case AST_NODE_STRUCT_DECLARATION:
            if (visitor->visitStructDeclaration) {
                visitor->visitStructDeclaration(visitor, (StructDeclaration*)node);
            }
            break;

        case AST_NODE_UNION_DECLARATION:
            if (visitor->visitUnionDeclaration) {
                visitor->visitUnionDeclaration(visitor, (UnionDeclaration*)node);
            }
            break;

        case AST_NODE_ENUM_DECLARATION:
            if (visitor->visitEnumDeclaration) {
                visitor->visitEnumDeclaration(visitor, (EnumDeclaration*)node);
            }
            break;

        case AST_NODE_TYPEDEF_DECLARATION:
            if (visitor->visitTypedefDeclaration) {
                visitor->visitTypedefDeclaration(visitor, (TypedefDeclaration*)node);
            }
            break;

        // 类型说明符节点
        case AST_NODE_BASIC_TYPE_SPECIFIER:
            if (visitor->visitBasicTypeSpecifier) {
                visitor->visitBasicTypeSpecifier(visitor, (BasicTypeSpecifier*)node);
            }
            break;

        case AST_NODE_POINTER_TYPE_SPECIFIER:
            if (visitor->visitPointerTypeSpecifier) {
                visitor->visitPointerTypeSpecifier(visitor, (PointerTypeSpecifier*)node);
            }
            break;

        case AST_NODE_ARRAY_TYPE_SPECIFIER:
            if (visitor->visitArrayTypeSpecifier) {
                visitor->visitArrayTypeSpecifier(visitor, (ArrayTypeSpecifier*)node);
            }
            break;

        case AST_NODE_FUNCTION_TYPE_SPECIFIER:
            if (visitor->visitFunctionTypeSpecifier) {
                visitor->visitFunctionTypeSpecifier(visitor, (FunctionTypeSpecifier*)node);
            }
            break;

        case AST_NODE_STRUCT_TYPE_SPECIFIER:
            if (visitor->visitStructTypeSpecifier) {
                visitor->visitStructTypeSpecifier(visitor, (StructTypeSpecifier*)node);
            }
            break;

        case AST_NODE_UNION_TYPE_SPECIFIER:
            if (visitor->visitUnionTypeSpecifier) {
                visitor->visitUnionTypeSpecifier(visitor, (UnionTypeSpecifier*)node);
            }
            break;

        case AST_NODE_ENUM_TYPE_SPECIFIER:
            if (visitor->visitEnumTypeSpecifier) {
                visitor->visitEnumTypeSpecifier(visitor, (EnumTypeSpecifier*)node);
            }
            break;

        case AST_NODE_TYPEDEF_NAME_SPECIFIER:
            if (visitor->visitTypedefNameSpecifier) {
                visitor->visitTypedefNameSpecifier(visitor, (TypedefNameSpecifier*)node);
            }
            break;

        default:
            // 未知节点类型，调用通用visit函数
            if (visitor->visit) {
                visitor->visit(visitor, node);
            }
            break;
    }

    // 调用访问后钩子
    if (visitor->afterVisit) {
        visitor->afterVisit(visitor, node);
    }
}

// ==================== 遍历辅助函数实现 ====================

/**
 * @brief 内部递归深度优先遍历
 */
static void traverseDFSRecursive(ASTNode* node, ASTVisitor* visitor,
                                  ASTTraversalContext* context, bool preorder) {
    if (!node || !visitor || !context) {
        return;
    }

    // 检查深度限制
    if (context->maxDepth > 0 && context->depth >= context->maxDepth) {
        return;
    }

    // 检查是否停止遍历
    if (context->stopTraversal) {
        return;
    }

    context->depth++;

    // 前序遍历：先访问当前节点
    if (preorder) {
        astNodeAccept(node, visitor);
    }

    // 访问子节点
    if (context->visitChildren) {
        switch (node->nodeType) {
            case AST_NODE_TRANSLATION_UNIT: {
                TranslationUnit* unit = (TranslationUnit*)node;
                if (unit->declarations) {
                    for (size_t i = 0; i < vectorSize(unit->declarations); i++) {
                        ASTNode* child = (ASTNode*)vectorGet(unit->declarations, i);
                        traverseDFSRecursive(child, visitor, context, preorder);
                    }
                }
                break;
            }

            case AST_NODE_COMPOUND_STATEMENT: {
                CompoundStatement* stmt = (CompoundStatement*)node;
                if (stmt->statements) {
                    for (size_t i = 0; i < vectorSize(stmt->statements); i++) {
                        ASTNode* child = (ASTNode*)vectorGet(stmt->statements, i);
                        traverseDFSRecursive(child, visitor, context, preorder);
                    }
                }
                if (stmt->declarations) {
                    for (size_t i = 0; i < vectorSize(stmt->declarations); i++) {
                        ASTNode* child = (ASTNode*)vectorGet(stmt->declarations, i);
                        traverseDFSRecursive(child, visitor, context, preorder);
                    }
                }
                break;
            }

            // 其他复合节点的子节点访问...
            default:
                break;
        }
    }

    // 后序遍历：后访问当前节点
    if (!preorder) {
        astNodeAccept(node, visitor);
    }

    context->depth--;
}

void astTraverseDFS(ASTNode* root, ASTVisitor* visitor, bool preorder) {
    if (!root || !visitor) {
        return;
    }

    ASTTraversalContext* context = createASTTraversalContext();
    if (!context) {
        return;
    }

    context->visitChildren = true;
    traverseDFSRecursive(root, visitor, context, preorder);

    destroyASTTraversalContext(context);
}

void astTraverseBFS(ASTNode* root, ASTVisitor* visitor) {
    if (!root || !visitor) {
        return;
    }

    // 使用队列实现广度优先遍历
    Vector* queue = vectorCreate(sizeof(ASTNode*), 4);
    if (!queue) {
        return;
    }

    vectorPushBack(queue, &root);

    while (vectorSize(queue) > 0) {
        ASTNode* node = *(ASTNode**)vectorGet(queue, 0);
        vectorEraseAt(queue, 0);

        // 访问当前节点
        astNodeAccept(node, visitor);

        // 将子节点加入队列
        switch (node->nodeType) {
            case AST_NODE_TRANSLATION_UNIT: {
                TranslationUnit* unit = (TranslationUnit*)node;
                if (unit->declarations) {
                    for (size_t i = 0; i < vectorSize(unit->declarations); i++) {
                        ASTNode* child = (ASTNode*)vectorGet(unit->declarations, i);
                        vectorPushBack(queue, &child);
                    }
                }
                break;
            }

            case AST_NODE_COMPOUND_STATEMENT: {
                CompoundStatement* stmt = (CompoundStatement*)node;
                if (stmt->statements) {
                    for (size_t i = 0; i < vectorSize(stmt->statements); i++) {
                        ASTNode* child = (ASTNode*)vectorGet(stmt->statements, i);
                        vectorPushBack(queue, &child);
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    vectorDestroy(queue, NULL);
}

void astTraverseChildren(Vector* children, ASTVisitor* visitor) {
    if (!children || !visitor) {
        return;
    }

    for (size_t i = 0; i < vectorSize(children); i++) {
        ASTNode* child = (ASTNode*)vectorGet(children, i);
        astNodeAccept(child, visitor);
    }
}

// ==================== 访问者工厂函数实现 ====================

ASTVisitor* createASTVisitor(void) {
    ASTVisitor* visitor = (ASTVisitor*)calloc(1, sizeof(ASTVisitor));
    if (!visitor) {
        return NULL;
    }

    // 所有函数指针初始化为NULL
    return visitor;
}

void destroyASTVisitor(ASTVisitor* visitor) {
    if (!visitor) {
        return;
    }

    // 注意：不释放visitor->privateData，由调用者负责
    free(visitor);
}

ASTVisitor* copyASTVisitor(const ASTVisitor* src) {
    if (!src) {
        return NULL;
    }

    ASTVisitor* visitor = (ASTVisitor*)malloc(sizeof(ASTVisitor));
    if (!visitor) {
        return NULL;
    }

    // 浅拷贝所有字段
    memcpy(visitor, src, sizeof(ASTVisitor));

    return visitor;
}

// ==================== 遍历上下文辅助结构实现 ====================

ASTTraversalContext* createASTTraversalContext(void) {
    ASTTraversalContext* context = (ASTTraversalContext*)calloc(1, sizeof(ASTTraversalContext));
    if (!context) {
        return NULL;
    }

    context->depth = 0;
    context->maxDepth = 0;  // 0表示无限制
    context->visitChildren = true;
    context->stopTraversal = false;
    context->userData = NULL;

    return context;
}

void destroyASTTraversalContext(ASTTraversalContext* context) {
    if (!context) {
        return;
    }

    // 注意：不释放context->userData，由调用者负责
    free(context);
}

void resetASTTraversalContext(ASTTraversalContext* context) {
    if (!context) {
        return;
    }

    context->depth = 0;
    context->visitChildren = true;
    context->stopTraversal = false;
}

// ==================== 内省辅助函数实现 ====================

const char* astVisitorFunctionName(ASTNodeType nodeType) {
    return getVisitorFunctionName(nodeType);
}

bool astVisitorHasHandler(const ASTVisitor* visitor, ASTNodeType nodeType) {
    if (!visitor) {
        return false;
    }

    switch (nodeType) {
        case AST_NODE_TRANSLATION_UNIT:
            return visitor->visitTranslationUnit != NULL;

        case AST_NODE_LITERAL_EXPR:
            return visitor->visitLiteralExpr != NULL;
        case AST_NODE_IDENTIFIER_EXPR:
            return visitor->visitIdentifierExpr != NULL;
        case AST_NODE_BINARY_OPERATOR_EXPR:
            return visitor->visitBinaryOperatorExpr != NULL;
        case AST_NODE_UNARY_OPERATOR_EXPR:
            return visitor->visitUnaryOperatorExpr != NULL;
        case AST_NODE_ASSIGNMENT_EXPR:
            return visitor->visitAssignmentExpr != NULL;
        case AST_NODE_TERNARY_EXPR:
            return visitor->visitTernaryExpr != NULL;
        case AST_NODE_FUNCTION_CALL_EXPR:
            return visitor->visitFunctionCallExpr != NULL;
        case AST_NODE_ARRAY_SUBSCRIPT_EXPR:
            return visitor->visitArraySubscriptExpr != NULL;
        case AST_NODE_MEMBER_ACCESS_EXPR:
            return visitor->visitMemberAccessExpr != NULL;
        case AST_NODE_CAST_EXPR:
            return visitor->visitCastExpr != NULL;

        case AST_NODE_EXPRESSION_STATEMENT:
            return visitor->visitExpressionStatement != NULL;
        case AST_NODE_COMPOUND_STATEMENT:
            return visitor->visitCompoundStatement != NULL;
        case AST_NODE_IF_STATEMENT:
            return visitor->visitIfStatement != NULL;
        case AST_NODE_WHILE_STATEMENT:
            return visitor->visitWhileStatement != NULL;
        case AST_NODE_DO_WHILE_STATEMENT:
            return visitor->visitDoWhileStatement != NULL;
        case AST_NODE_FOR_STATEMENT:
            return visitor->visitForStatement != NULL;
        case AST_NODE_RETURN_STATEMENT:
            return visitor->visitReturnStatement != NULL;
        case AST_NODE_BREAK_STATEMENT:
            return visitor->visitBreakStatement != NULL;
        case AST_NODE_CONTINUE_STATEMENT:
            return visitor->visitContinueStatement != NULL;
        case AST_NODE_SWITCH_STATEMENT:
            return visitor->visitSwitchStatement != NULL;
        case AST_NODE_CASE_STATEMENT:
            return visitor->visitCaseStatement != NULL;
        case AST_NODE_LABELED_STATEMENT:
            return visitor->visitLabeledStatement != NULL;
        case AST_NODE_GOTO_STATEMENT:
            return visitor->visitGotoStatement != NULL;

        case AST_NODE_VARIABLE_DECLARATION:
            return visitor->visitVariableDeclaration != NULL;
        case AST_NODE_FUNCTION_DECLARATION:
            return visitor->visitFunctionDeclaration != NULL;
        case AST_NODE_STRUCT_DECLARATION:
            return visitor->visitStructDeclaration != NULL;
        case AST_NODE_UNION_DECLARATION:
            return visitor->visitUnionDeclaration != NULL;
        case AST_NODE_ENUM_DECLARATION:
            return visitor->visitEnumDeclaration != NULL;
        case AST_NODE_TYPEDEF_DECLARATION:
            return visitor->visitTypedefDeclaration != NULL;

        case AST_NODE_BASIC_TYPE_SPECIFIER:
            return visitor->visitBasicTypeSpecifier != NULL;
        case AST_NODE_POINTER_TYPE_SPECIFIER:
            return visitor->visitPointerTypeSpecifier != NULL;
        case AST_NODE_ARRAY_TYPE_SPECIFIER:
            return visitor->visitArrayTypeSpecifier != NULL;
        case AST_NODE_FUNCTION_TYPE_SPECIFIER:
            return visitor->visitFunctionTypeSpecifier != NULL;
        case AST_NODE_STRUCT_TYPE_SPECIFIER:
            return visitor->visitStructTypeSpecifier != NULL;
        case AST_NODE_UNION_TYPE_SPECIFIER:
            return visitor->visitUnionTypeSpecifier != NULL;
        case AST_NODE_ENUM_TYPE_SPECIFIER:
            return visitor->visitEnumTypeSpecifier != NULL;
        case AST_NODE_TYPEDEF_NAME_SPECIFIER:
            return visitor->visitTypedefNameSpecifier != NULL;

        default:
            return visitor->visit != NULL;
    }
}

int astVisitorHandlerCount(const ASTVisitor* visitor) {
    if (!visitor) {
        return 0;
    }

    int count = 0;

    // 检查所有访问函数指针
    if (visitor->visitTranslationUnit) count++;

    if (visitor->visitLiteralExpr) count++;
    if (visitor->visitIdentifierExpr) count++;
    if (visitor->visitBinaryOperatorExpr) count++;
    if (visitor->visitUnaryOperatorExpr) count++;
    if (visitor->visitAssignmentExpr) count++;
    if (visitor->visitTernaryExpr) count++;
    if (visitor->visitFunctionCallExpr) count++;
    if (visitor->visitArraySubscriptExpr) count++;
    if (visitor->visitMemberAccessExpr) count++;
    if (visitor->visitCastExpr) count++;

    if (visitor->visitExpressionStatement) count++;
    if (visitor->visitCompoundStatement) count++;
    if (visitor->visitIfStatement) count++;
    if (visitor->visitWhileStatement) count++;
    if (visitor->visitDoWhileStatement) count++;
    if (visitor->visitForStatement) count++;
    if (visitor->visitReturnStatement) count++;
    if (visitor->visitBreakStatement) count++;
    if (visitor->visitContinueStatement) count++;
    if (visitor->visitSwitchStatement) count++;
    if (visitor->visitCaseStatement) count++;
    if (visitor->visitLabeledStatement) count++;
    if (visitor->visitGotoStatement) count++;

    if (visitor->visitVariableDeclaration) count++;
    if (visitor->visitFunctionDeclaration) count++;
    if (visitor->visitStructDeclaration) count++;
    if (visitor->visitUnionDeclaration) count++;
    if (visitor->visitEnumDeclaration) count++;
    if (visitor->visitTypedefDeclaration) count++;

    if (visitor->visitBasicTypeSpecifier) count++;
    if (visitor->visitPointerTypeSpecifier) count++;
    if (visitor->visitArrayTypeSpecifier) count++;
    if (visitor->visitFunctionTypeSpecifier) count++;
    if (visitor->visitStructTypeSpecifier) count++;
    if (visitor->visitUnionTypeSpecifier) count++;
    if (visitor->visitEnumTypeSpecifier) count++;
    if (visitor->visitTypedefNameSpecifier) count++;

    if (visitor->visit) count++;

    return count;
}
