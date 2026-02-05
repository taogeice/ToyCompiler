/**
 * @file ast_utils.c
 * @brief AST工具函数实现
 *
 * 提供AST操作的通用工具函数。
 */

#include "ast_utils.h"
#include "../../common/diagnostics/diagnostic_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================== 内部辅助函数 ====================

/**
 * @brief 递归计数节点
 */
static size_t countNodesRecursive(const ASTNode* node) {
    if (!node) {
        return 0;
    }

    size_t count = 1;  // 计数当前节点

    // 根据节点类型递归计数子节点
    switch (node->nodeType) {
        case AST_NODE_TRANSLATION_UNIT: {
            const TranslationUnit* unit = (const TranslationUnit*)node;
            if (unit->declarations) {
                for (size_t i = 0; i < vectorSize(unit->declarations); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(unit->declarations, i);
                    count += countNodesRecursive(child);
                }
            }
            break;
        }

        case AST_NODE_COMPOUND_STATEMENT: {
            const CompoundStatement* stmt = (const CompoundStatement*)node;
            if (stmt->statements) {
                for (size_t i = 0; i < vectorSize(stmt->statements); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(stmt->statements, i);
                    count += countNodesRecursive(child);
                }
            }
            if (stmt->declarations) {
                for (size_t i = 0; i < vectorSize(stmt->declarations); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(stmt->declarations, i);
                    count += countNodesRecursive(child);
                }
            }
            break;
        }

        case AST_NODE_BINARY_OPERATOR_EXPR: {
            const BinaryOperatorExpr* expr = (const BinaryOperatorExpr*)node;
            if (expr->left) count += countNodesRecursive((ASTNode*)expr->left);
            if (expr->right) count += countNodesRecursive((ASTNode*)expr->right);
            break;
        }

        case AST_NODE_UNARY_OPERATOR_EXPR: {
            const UnaryOperatorExpr* expr = (const UnaryOperatorExpr*)node;
            if (expr->operand) count += countNodesRecursive((ASTNode*)expr->operand);
            break;
        }

        case AST_NODE_ASSIGNMENT_EXPR: {
            const AssignmentExpr* expr = (const AssignmentExpr*)node;
            if (expr->left) count += countNodesRecursive((ASTNode*)expr->left);
            if (expr->right) count += countNodesRecursive((ASTNode*)expr->right);
            break;
        }

        case AST_NODE_TERNARY_EXPR: {
            const TernaryExpr* expr = (const TernaryExpr*)node;
            if (expr->condition) count += countNodesRecursive((ASTNode*)expr->condition);
            if (expr->thenExpr) count += countNodesRecursive((ASTNode*)expr->thenExpr);
            if (expr->elseExpr) count += countNodesRecursive((ASTNode*)expr->elseExpr);
            break;
        }

        case AST_NODE_FUNCTION_CALL_EXPR: {
            const FunctionCallExpr* expr = (const FunctionCallExpr*)node;
            if (expr->callee) count += countNodesRecursive((ASTNode*)expr->callee);
            if (expr->arguments) {
                for (size_t i = 0; i < vectorSize(expr->arguments); i++) {
                    Expression* arg = (Expression*)vectorGet(expr->arguments, i);
                    count += countNodesRecursive((ASTNode*)arg);
                }
            }
            break;
        }

        case AST_NODE_ARRAY_SUBSCRIPT_EXPR: {
            const ArraySubscriptExpr* expr = (const ArraySubscriptExpr*)node;
            if (expr->array) count += countNodesRecursive((ASTNode*)expr->array);
            if (expr->index) count += countNodesRecursive((ASTNode*)expr->index);
            break;
        }

        case AST_NODE_MEMBER_ACCESS_EXPR: {
            const MemberAccessExpr* expr = (const MemberAccessExpr*)node;
            if (expr->baseExpr) count += countNodesRecursive((ASTNode*)expr->baseExpr);
            break;
        }

        case AST_NODE_CAST_EXPR: {
            const CastExpr* expr = (const CastExpr*)node;
            if (expr->targetType) count += countNodesRecursive((ASTNode*)expr->targetType);
            if (expr->operand) count += countNodesRecursive((ASTNode*)expr->operand);
            break;
        }

        case AST_NODE_EXPRESSION_STATEMENT: {
            const ExpressionStatement* stmt = (const ExpressionStatement*)node;
            if (stmt->expression) count += countNodesRecursive((ASTNode*)stmt->expression);
            break;
        }

        case AST_NODE_IF_STATEMENT: {
            const IfStatement* stmt = (const IfStatement*)node;
            if (stmt->condition) count += countNodesRecursive((ASTNode*)stmt->condition);
            if (stmt->thenStmt) count += countNodesRecursive((ASTNode*)stmt->thenStmt);
            if (stmt->elseStmt) count += countNodesRecursive((ASTNode*)stmt->elseStmt);
            break;
        }

        case AST_NODE_WHILE_STATEMENT: {
            const WhileStatement* stmt = (const WhileStatement*)node;
            if (stmt->condition) count += countNodesRecursive((ASTNode*)stmt->condition);
            if (stmt->body) count += countNodesRecursive((ASTNode*)stmt->body);
            break;
        }

        case AST_NODE_DO_WHILE_STATEMENT: {
            const DoWhileStatement* stmt = (const DoWhileStatement*)node;
            if (stmt->body) count += countNodesRecursive((ASTNode*)stmt->body);
            if (stmt->condition) count += countNodesRecursive((ASTNode*)stmt->condition);
            break;
        }

        case AST_NODE_FOR_STATEMENT: {
            const ForStatement* stmt = (const ForStatement*)node;
            if (stmt->init) count += countNodesRecursive((ASTNode*)stmt->init);
            if (stmt->condition) count += countNodesRecursive((ASTNode*)stmt->condition);
            if (stmt->increment) count += countNodesRecursive((ASTNode*)stmt->increment);
            if (stmt->body) count += countNodesRecursive((ASTNode*)stmt->body);
            break;
        }

        case AST_NODE_RETURN_STATEMENT: {
            const ReturnStatement* stmt = (const ReturnStatement*)node;
            if (stmt->returnValue) count += countNodesRecursive((ASTNode*)stmt->returnValue);
            break;
        }

        case AST_NODE_SWITCH_STATEMENT: {
            const SwitchStatement* stmt = (const SwitchStatement*)node;
            if (stmt->condition) count += countNodesRecursive((ASTNode*)stmt->condition);
            if (stmt->cases) {
                for (size_t i = 0; i < vectorSize(stmt->cases); i++) {
                    ASTNode* caseStmt = (ASTNode*)vectorGet(stmt->cases, i);
                    count += countNodesRecursive(caseStmt);
                }
            }
            break;
        }

        case AST_NODE_CASE_STATEMENT: {
            const CaseStatement* stmt = (const CaseStatement*)node;
            if (stmt->value) count += countNodesRecursive((ASTNode*)stmt->value);
            if (stmt->statement) count += countNodesRecursive((ASTNode*)stmt->statement);
            break;
        }

        case AST_NODE_LABELED_STATEMENT: {
            const LabeledStatement* stmt = (const LabeledStatement*)node;
            if (stmt->statement) count += countNodesRecursive((ASTNode*)stmt->statement);
            break;
        }

        case AST_NODE_VARIABLE_DECLARATION: {
            const VariableDeclaration* decl = (const VariableDeclaration*)node;
            if (decl->type) count += countNodesRecursive((ASTNode*)decl->type);
            if (decl->initializer) count += countNodesRecursive((ASTNode*)decl->initializer);
            break;
        }

        case AST_NODE_FUNCTION_DECLARATION: {
            const FunctionDeclaration* decl = (const FunctionDeclaration*)node;
            if (decl->returnType) count += countNodesRecursive((ASTNode*)decl->returnType);
            if (decl->parameters) {
                for (size_t i = 0; i < vectorSize(decl->parameters); i++) {
                    ASTNode* param = (ASTNode*)vectorGet(decl->parameters, i);
                    count += countNodesRecursive(param);
                }
            }
            if (decl->body) count += countNodesRecursive((ASTNode*)decl->body);
            break;
        }

        case AST_NODE_STRUCT_DECLARATION: {
            const StructDeclaration* decl = (const StructDeclaration*)node;
            if (decl->members) {
                for (size_t i = 0; i < vectorSize(decl->members); i++) {
                    ASTNode* member = (ASTNode*)vectorGet(decl->members, i);
                    count += countNodesRecursive(member);
                }
            }
            break;
        }

        case AST_NODE_UNION_DECLARATION: {
            const UnionDeclaration* decl = (const UnionDeclaration*)node;
            if (decl->members) {
                for (size_t i = 0; i < vectorSize(decl->members); i++) {
                    ASTNode* member = (ASTNode*)vectorGet(decl->members, i);
                    count += countNodesRecursive(member);
                }
            }
            break;
        }

        case AST_NODE_TYPEDEF_DECLARATION: {
            const TypedefDeclaration* decl = (const TypedefDeclaration*)node;
            if (decl->aliasedType) count += countNodesRecursive((ASTNode*)decl->aliasedType);
            break;
        }

        case AST_NODE_POINTER_TYPE_SPECIFIER: {
            const PointerTypeSpecifier* type = (const PointerTypeSpecifier*)node;
            if (type->baseType) count += countNodesRecursive((ASTNode*)type->baseType);
            break;
        }

        case AST_NODE_ARRAY_TYPE_SPECIFIER: {
            const ArrayTypeSpecifier* type = (const ArrayTypeSpecifier*)node;
            if (type->elementType) count += countNodesRecursive((ASTNode*)type->elementType);
            if (type->size) count += countNodesRecursive((ASTNode*)type->size);
            break;
        }

        case AST_NODE_FUNCTION_TYPE_SPECIFIER: {
            const FunctionTypeSpecifier* type = (const FunctionTypeSpecifier*)node;
            if (type->returnType) count += countNodesRecursive((ASTNode*)type->returnType);
            if (type->parameterTypes) {
                for (size_t i = 0; i < vectorSize(type->parameterTypes); i++) {
                    ASTNode* paramType = (ASTNode*)vectorGet(type->parameterTypes, i);
                    count += countNodesRecursive(paramType);
                }
            }
            break;
        }

        default:
            // 其他节点类型没有子节点
            break;
    }

    return count;
}

/**
 * @brief 递归计算树深度
 */
static size_t calculateDepthRecursive(const ASTNode* node, size_t currentDepth) {
    if (!node) {
        return currentDepth;
    }

    size_t maxDepth = currentDepth;

    // 根据节点类型递归计算深度
    switch (node->nodeType) {
        case AST_NODE_TRANSLATION_UNIT: {
            const TranslationUnit* unit = (const TranslationUnit*)node;
            if (unit->declarations) {
                for (size_t i = 0; i < vectorSize(unit->declarations); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(unit->declarations, i);
                    size_t childDepth = calculateDepthRecursive(child, currentDepth + 1);
                    if (childDepth > maxDepth) {
                        maxDepth = childDepth;
                    }
                }
            }
            break;
        }

        case AST_NODE_COMPOUND_STATEMENT: {
            const CompoundStatement* stmt = (const CompoundStatement*)node;
            if (stmt->statements) {
                for (size_t i = 0; i < vectorSize(stmt->statements); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(stmt->statements, i);
                    size_t childDepth = calculateDepthRecursive(child, currentDepth + 1);
                    if (childDepth > maxDepth) {
                        maxDepth = childDepth;
                    }
                }
            }
            break;
        }

        // 可以添加更多节点类型的处理...

        default:
            break;
    }

    return maxDepth;
}

// ==================== 节点信息函数 ====================

size_t astNodeCountDescendants(const ASTNode* node) {
    return countNodesRecursive(node);
}

size_t astTreeGetDepth(const ASTNode* root) {
    return calculateDepthRecursive(root, 0);
}

bool astNodeHasParent(const ASTNode* node) {
    return node && node->parent != NULL;
}

bool astNodeIsRoot(const ASTNode* node) {
    return node && node->parent == NULL;
}

size_t astNodeGetChildCount(const ASTNode* node) {
    if (!node) {
        return 0;
    }

    switch (node->nodeType) {
        case AST_NODE_TRANSLATION_UNIT: {
            const TranslationUnit* unit = (const TranslationUnit*)node;
            return unit->declarations ? vectorSize(unit->declarations) : 0;
        }

        case AST_NODE_COMPOUND_STATEMENT: {
            const CompoundStatement* stmt = (const CompoundStatement*)node;
            size_t count = 0;
            if (stmt->declarations) count += vectorSize(stmt->declarations);
            if (stmt->statements) count += vectorSize(stmt->statements);
            return count;
        }

        case AST_NODE_BINARY_OPERATOR_EXPR:
        case AST_NODE_ASSIGNMENT_EXPR:
            return 2;  // left and right

        case AST_NODE_TERNARY_EXPR:
            return 3;  // condition, thenExpr, elseExpr

        case AST_NODE_UNARY_OPERATOR_EXPR:
        case AST_NODE_CAST_EXPR:
        case AST_NODE_ARRAY_SUBSCRIPT_EXPR:
        case AST_NODE_MEMBER_ACCESS_EXPR:
            return 1;  // single operand

        case AST_NODE_FUNCTION_CALL_EXPR: {
            const FunctionCallExpr* expr = (const FunctionCallExpr*)node;
            size_t count = 1;  // callee
            if (expr->arguments) count += vectorSize(expr->arguments);
            return count;
        }

        default:
            return 0;
    }
}

// ==================== 节点遍历函数 ====================

void astNodeTraverse(ASTNode* root, ASTVisitor* visitor) {
    if (!root || !visitor) {
        return;
    }

    astNodeAccept(root, visitor);
}

// ==================== 节点查找函数 ====================

Vector* astNodeFindChildrenByType(const ASTNode* node, ASTNodeType type) {
    if (!node) {
        return NULL;
    }

    Vector* results = vectorCreate(sizeof(ASTNode*), 8);
    if (!results) {
        return NULL;
    }

    switch (node->nodeType) {
        case AST_NODE_TRANSLATION_UNIT: {
            const TranslationUnit* unit = (const TranslationUnit*)node;
            if (unit->declarations) {
                for (size_t i = 0; i < vectorSize(unit->declarations); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(unit->declarations, i);
                    if (child->nodeType == type) {
                        vectorPushBack(results, &child);
                    }
                }
            }
            break;
        }

        case AST_NODE_COMPOUND_STATEMENT: {
            const CompoundStatement* stmt = (const CompoundStatement*)node;
            if (stmt->statements) {
                for (size_t i = 0; i < vectorSize(stmt->statements); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(stmt->statements, i);
                    if (child->nodeType == type) {
                        vectorPushBack(results, &child);
                    }
                }
            }
            if (stmt->declarations) {
                for (size_t i = 0; i < vectorSize(stmt->declarations); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(stmt->declarations, i);
                    if (child->nodeType == type) {
                        vectorPushBack(results, &child);
                    }
                }
            }
            break;
        }

        default:
            break;
    }

    return results;
}

ASTNode* astNodeFindParentByType(const ASTNode* node, ASTNodeType type) {
    if (!node) {
        return NULL;
    }

    ASTNode* current = node->parent;
    while (current != NULL) {
        if (current->nodeType == type) {
            return current;
        }
        current = current->parent;
    }

    return NULL;
}

Vector* astNodeFindAllByType(ASTNode* root, ASTNodeType type) {
    if (!root) {
        return NULL;
    }

    Vector* results = vectorCreate(sizeof(ASTNode*), 8);
    if (!results) {
        return NULL;
    }

    // 使用栈进行深度优先搜索
    Vector* stack = vectorCreate(sizeof(ASTNode*), 8);
    if (!stack) {
        vectorDestroy(results, NULL);
        return NULL;
    }

    vectorPushBack(stack, &root);

    while (vectorSize(stack) > 0) {
        ASTNode* current = *(ASTNode**)vectorGet(stack, vectorSize(stack) - 1);
        vectorPopBack(stack, NULL);

        if (current->nodeType == type) {
            vectorPushBack(results, &current);
        }

        // 添加子节点到栈
        switch (current->nodeType) {
            case AST_NODE_TRANSLATION_UNIT: {
                const TranslationUnit* unit = (const TranslationUnit*)current;
                if (unit->declarations) {
                    for (size_t i = 0; i < vectorSize(unit->declarations); i++) {
                        ASTNode* child = (ASTNode*)vectorGet(unit->declarations, i);
                        vectorPushBack(stack, &child);
                    }
                }
                break;
            }

            case AST_NODE_COMPOUND_STATEMENT: {
                const CompoundStatement* stmt = (const CompoundStatement*)current;
                if (stmt->statements) {
                    for (size_t i = 0; i < vectorSize(stmt->statements); i++) {
                        ASTNode* child = (ASTNode*)vectorGet(stmt->statements, i);
                        vectorPushBack(stack, &child);
                    }
                }
                if (stmt->declarations) {
                    for (size_t i = 0; i < vectorSize(stmt->declarations); i++) {
                        ASTNode* child = (ASTNode*)vectorGet(stmt->declarations, i);
                        vectorPushBack(stack, &child);
                    }
                }
                break;
            }

            // 可以添加更多节点类型的处理...

            default:
                break;
        }
    }

    vectorDestroy(stack, NULL);
    return results;
}

// ==================== 节点验证函数 ====================

bool astNodeValidate(const ASTNode* node, DiagnosticEngine* diagnostics) {
    if (!node) {
        return false;
    }

    bool valid = true;

    // 验证父节点指针一致性
    if (node->parent) {
        // TODO: 验证父节点确实包含此节点作为子节点
    }

    // 根据节点类型进行特定验证
    switch (node->nodeType) {
        case AST_NODE_LITERAL_EXPR: {
            const LiteralExpr* expr = (const LiteralExpr*)node;
            if (expr->literalToken->type == TOKEN_UNKNOWN) {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "字面量表达式token类型无效",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            break;
        }

        case AST_NODE_IDENTIFIER_EXPR: {
            const IdentifierExpr* expr = (const IdentifierExpr*)node;
            if (!expr->name || expr->name[0] == '\0') {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "标识符表达式名称为空",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            break;
        }

        case AST_NODE_BINARY_OPERATOR_EXPR: {
            const BinaryOperatorExpr* expr = (const BinaryOperatorExpr*)node;
            if (!expr->left || !expr->right) {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "二元运算符表达式缺少操作数",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            break;
        }

        case AST_NODE_ASSIGNMENT_EXPR: {
            const AssignmentExpr* expr = (const AssignmentExpr*)node;
            if (!expr->left || !expr->right) {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "赋值表达式缺少操作数",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            // TODO: 验证left是左值
            break;
        }

        case AST_NODE_FUNCTION_CALL_EXPR: {
            const FunctionCallExpr* expr = (const FunctionCallExpr*)node;
            if (!expr->callee) {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "函数调用表达式缺少被调用者",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            break;
        }

        case AST_NODE_IF_STATEMENT: {
            const IfStatement* stmt = (const IfStatement*)node;
            if (!stmt->condition || !stmt->thenStmt) {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "if语句缺少条件或then分支",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            break;
        }

        case AST_NODE_WHILE_STATEMENT: {
            const WhileStatement* stmt = (const WhileStatement*)node;
            if (!stmt->condition || !stmt->body) {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "while语句缺少条件或循环体",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            break;
        }

        case AST_NODE_FOR_STATEMENT: {
            const ForStatement* stmt = (const ForStatement*)node;
            if (!stmt->body) {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "for语句缺少循环体",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            break;
        }

        case AST_NODE_VARIABLE_DECLARATION: {
            const VariableDeclaration* decl = (const VariableDeclaration*)node;
            if (!decl->base.name || decl->base.name[0] == '\0') {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "变量声明名称为空",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            if (!decl->type) {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "变量声明缺少类型",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            break;
        }

        case AST_NODE_FUNCTION_DECLARATION: {
            const FunctionDeclaration* decl = (const FunctionDeclaration*)node;
            if (!decl->base.name || decl->base.name[0] == '\0') {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "函数声明名称为空",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            if (!decl->returnType) {
                if (diagnostics) {
                    diagnosticEngineEmitDiagnostic(diagnostics,
                        DIAGNOSTIC_ERROR,
                        "函数声明缺少返回类型",
                        node->location,
                        NULL,
                        0);
                }
                valid = false;
            }
            break;
        }

        default:
            break;
    }

    return valid;
}

bool astTreeValidate(const ASTNode* root, DiagnosticEngine* diagnostics) {
    if (!root) {
        return false;
    }

    bool valid = true;

    // 验证根节点
    if (!astNodeValidate(root, diagnostics)) {
        valid = false;
    }

    // 递归验证所有子节点
    switch (root->nodeType) {
        case AST_NODE_TRANSLATION_UNIT: {
            const TranslationUnit* unit = (const TranslationUnit*)root;
            if (unit->declarations) {
                for (size_t i = 0; i < vectorSize(unit->declarations); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(unit->declarations, i);
                    if (!astTreeValidate(child, diagnostics)) {
                        valid = false;
                    }
                }
            }
            break;
        }

        case AST_NODE_COMPOUND_STATEMENT: {
            const CompoundStatement* stmt = (const CompoundStatement*)root;
            if (stmt->statements) {
                for (size_t i = 0; i < vectorSize(stmt->statements); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(stmt->statements, i);
                    if (!astTreeValidate(child, diagnostics)) {
                        valid = false;
                    }
                }
            }
            if (stmt->declarations) {
                for (size_t i = 0; i < vectorSize(stmt->declarations); i++) {
                    ASTNode* child = (ASTNode*)vectorGet(stmt->declarations, i);
                    if (!astTreeValidate(child, diagnostics)) {
                        valid = false;
                    }
                }
            }
            break;
        }

        // 可以添加更多节点类型的处理...

        default:
            break;
    }

    return valid;
}

// ==================== 节点克隆函数 ====================

ASTNode* astNodeClone(const ASTNode* node) {
    if (!node) {
        return NULL;
    }

    // TODO: 实现深度克隆
    // 这需要根据节点类型复制所有字段和子节点

    return NULL;
}

// ==================== 节点比较函数 ====================

bool astNodeEquals(const ASTNode* a, const ASTNode* b) {
    if (!a || !b) {
        return a == b;
    }

    if (a->nodeType != b->nodeType) {
        return false;
    }

    // TODO: 实现深度比较
    // 这需要根据节点类型比较所有字段和子节点

    return false;
}
