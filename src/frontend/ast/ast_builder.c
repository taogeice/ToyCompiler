/**
 * @file ast_builder.c
 * @brief AST构建器实现
 *
 * 提供高级API用于构建AST节点，简化Parser集成。
 */

#include "ast_builder.h"
#include "../../common/diagnostics/diagnostic_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================== 内部辅助函数 ====================

/**
 * @brief 验证节点名称
 */
static bool validateNodeName(const char* name) {
    if (!name || name[0] == '\0') {
        return false;
    }

    // 检查名称是否以数字开头（不符合C标识符规则）
    if (name[0] >= '0' && name[0] <= '9') {
        return false;
    }

    // 检查名称中是否包含非法字符
    for (size_t i = 0; name[i] != '\0'; i++) {
        char c = name[i];
        bool isValid = (c >= 'a' && c <= 'z') ||
                       (c >= 'A' && c <= 'Z') ||
                       (c >= '0' && c <= '9') ||
                       c == '_';
        if (!isValid) {
            return false;
        }
    }

    return true;
}

/**
 * @brief 创建源位置
 */
static SourceLocation createLocation(DiagnosticEngine* diagnostics, int line, int column) {
    SourceLocation loc;
    loc.filename = NULL;
    loc.line = line;
    loc.column = column;
    loc.offset = 0;
    return loc;
}

// ==================== 构造函数和析构函数 ====================

ASTBuilder* createASTBuilder(DiagnosticEngine* diagnostics) {
    if (!diagnostics) {
        return NULL;
    }

    ASTBuilder* builder = (ASTBuilder*)calloc(1, sizeof(ASTBuilder));
    if (!builder) {
        diagnosticEngineEmitDiagnostic(diagnostics,
            DIAGNOSTIC_ERROR,
            "无法分配AST构建器内存",
            (SourceLocation){0},
            NULL,
            0);
        return NULL;
    }

    builder->diagnostics = diagnostics;
    builder->root = createTranslationUnit();
    builder->nodePool = vectorCreate(sizeof(ASTNode*), 4);
    builder->scopeStack = vectorCreate(sizeof(SymbolTable*), 4);

    if (!builder->root || !builder->nodePool || !builder->scopeStack) {
        // 清理已分配的资源
        if (builder->root) destroyTranslationUnit(builder->root);
        if (builder->nodePool) vectorDestroy(builder->nodePool, NULL);
        if (builder->scopeStack) vectorDestroy(builder->scopeStack, NULL);
        free(builder);
        return NULL;
    }

    return builder;
}

void destroyASTBuilder(ASTBuilder* builder) {
    if (!builder) {
        return;
    }

    // 销毁根节点
    if (builder->root) {
        destroyTranslationUnit(builder->root);
    }

    // 销毁节点池（注意：节点已被根节点销毁，这里只需要销毁容器）
    if (builder->nodePool) {
        vectorDestroy(builder->nodePool, NULL);
    }

    // 销毁作用域栈（注意：符号表不由我们管理，只销毁容器）
    if (builder->scopeStack) {
        vectorDestroy(builder->scopeStack, NULL);
    }

    free(builder);
}

TranslationUnit* astBuilderGetRoot(ASTBuilder* builder) {
    return builder ? builder->root : NULL;
}

DiagnosticEngine* astBuilderGetDiagnostics(ASTBuilder* builder) {
    return builder ? builder->diagnostics : NULL;
}

// ==================== 声明构建函数 ====================

Declaration* astBuilderAddVariableDecl(ASTBuilder* builder, const char* name,
                                        TypeSpecifier* type, Expression* initializer,
                                        SourceLocation location) {
    if (!builder || !name || !type) {
        return NULL;
    }

    // 验证名称
    if (!validateNodeName(name)) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无效的变量名称",
            location,
            "变量名称",
            0);
        return NULL;
    }

    // 创建变量声明
    Declaration* decl = createVariableDeclaration(name, type, initializer, location);
    if (!decl) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建变量声明",
            location,
            name,
            0);
        return NULL;
    }

    // 添加到翻译单元
    if (builder->root && builder->root->declarations) {
        vectorPushBack(builder->root->declarations, &decl);
    }

    return decl;
}

Declaration* astBuilderAddFunctionDecl(ASTBuilder* builder, const char* name,
                                        TypeSpecifier* returnType, Vector* parameters,
                                        CompoundStatement* body, SourceLocation location) {
    if (!builder || !name || !returnType) {
        return NULL;
    }

    // 验证名称
    if (!validateNodeName(name)) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无效的函数名称",
            location,
            "函数名称",
            0);
        return NULL;
    }

    // 创建函数声明
    Declaration* decl = createFunctionDeclaration(name, returnType, parameters, body, location);
    if (!decl) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建函数声明",
            location,
            name,
            0);
        return NULL;
    }

    // 添加到翻译单元
    if (builder->root && builder->root->declarations) {
        vectorPushBack(builder->root->declarations, &decl);
    }

    return decl;
}

Declaration* astBuilderAddStructDecl(ASTBuilder* builder, const char* name,
                                      Vector* members, SourceLocation location) {
    if (!builder || !members) {
        return NULL;
    }

    // 如果有名称，验证名称
    if (name && !validateNodeName(name)) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无效的结构体名称",
            location,
            "结构体名称",
            0);
        return NULL;
    }

    // 创建结构体声明
    Declaration* decl = createStructDeclaration(name, members, location);
    if (!decl) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建结构体声明",
            location,
            name ? name : "<anonymous>",
            0);
        return NULL;
    }

    // 添加到翻译单元
    if (builder->root && builder->root->declarations) {
        vectorPushBack(builder->root->declarations, &decl);
    }

    return decl;
}

Declaration* astBuilderAddUnionDecl(ASTBuilder* builder, const char* name,
                                     Vector* members, SourceLocation location) {
    if (!builder || !members) {
        return NULL;
    }

    // 如果有名称，验证名称
    if (name && !validateNodeName(name)) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无效的联合体名称",
            location,
            "联合体名称",
            0);
        return NULL;
    }

    // 创建联合体声明
    Declaration* decl = createUnionDeclaration(name, members, location);
    if (!decl) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建联合体声明",
            location,
            name ? name : "<anonymous>",
            0);
        return NULL;
    }

    // 添加到翻译单元
    if (builder->root && builder->root->declarations) {
        vectorPushBack(builder->root->declarations, &decl);
    }

    return decl;
}

Declaration* astBuilderAddEnumDecl(ASTBuilder* builder, const char* name,
                                    Vector* constants, SourceLocation location) {
    if (!builder || !constants) {
        return NULL;
    }

    // 如果有名称，验证名称
    if (name && !validateNodeName(name)) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无效的枚举名称",
            location,
            "枚举名称",
            0);
        return NULL;
    }

    // 创建枚举声明
    Declaration* decl = createEnumDeclaration(name, constants, location);
    if (!decl) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建枚举声明",
            location,
            name ? name : "<anonymous>",
            0);
        return NULL;
    }

    // 添加到翻译单元
    if (builder->root && builder->root->declarations) {
        vectorPushBack(builder->root->declarations, &decl);
    }

    return decl;
}

Declaration* astBuilderAddTypedefDecl(ASTBuilder* builder, const char* name,
                                       TypeSpecifier* aliasedType, SourceLocation location) {
    if (!builder || !name || !aliasedType) {
        return NULL;
    }

    // 验证名称
    if (!validateNodeName(name)) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无效的typedef名称",
            location,
            "typedef名称",
            0);
        return NULL;
    }

    // 创建typedef声明
    Declaration* decl = createTypedefDeclaration(name, aliasedType, location);
    if (!decl) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建typedef声明",
            location,
            name,
            0);
        return NULL;
    }

    // 添加到翻译单元
    if (builder->root && builder->root->declarations) {
        vectorPushBack(builder->root->declarations, &decl);
    }

    return decl;
}

// ==================== 语句构建函数 ====================

Statement* astBuilderCreateExprStmt(ASTBuilder* builder, Expression* expression,
                                      SourceLocation location) {
    if (!builder || !expression) {
        return NULL;
    }

    Statement* stmt = createExpressionStatement(expression, location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建表达式语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

Statement* astBuilderCreateCompoundStmt(ASTBuilder* builder, SourceLocation location) {
    if (!builder) {
        return NULL;
    }

    Statement* stmt = createCompoundStatement(location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建复合语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

Statement* astBuilderCreateIfStmt(ASTBuilder* builder, Expression* condition,
                                   Statement* thenStmt, Statement* elseStmt,
                                   SourceLocation location) {
    if (!builder || !condition || !thenStmt) {
        return NULL;
    }

    Statement* stmt = createIfStatement(condition, thenStmt, elseStmt, location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建if语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

Statement* astBuilderCreateWhileStmt(ASTBuilder* builder, Expression* condition,
                                      Statement* body, SourceLocation location) {
    if (!builder || !condition || !body) {
        return NULL;
    }

    Statement* stmt = createWhileStatement(condition, body, location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建while语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

Statement* astBuilderCreateDoWhileStmt(ASTBuilder* builder, Statement* body,
                                        Expression* condition, SourceLocation location) {
    if (!builder || !body || !condition) {
        return NULL;
    }

    Statement* stmt = createDoWhileStatement(body, condition, location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建do-while语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

Statement* astBuilderCreateForStmt(ASTBuilder* builder, Expression* init,
                                    Expression* condition, Expression* increment,
                                    Statement* body, SourceLocation location) {
    if (!builder || !body) {
        return NULL;
    }

    Statement* stmt = createForStatement(init, condition, increment, body, location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建for语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

Statement* astBuilderCreateReturnStmt(ASTBuilder* builder, Expression* returnValue,
                                       SourceLocation location) {
    if (!builder) {
        return NULL;
    }

    Statement* stmt = createReturnStatement(returnValue, location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建return语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

Statement* astBuilderCreateBreakStmt(ASTBuilder* builder, SourceLocation location) {
    if (!builder) {
        return NULL;
    }

    Statement* stmt = createBreakStatement(location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建break语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

Statement* astBuilderCreateContinueStmt(ASTBuilder* builder, SourceLocation location) {
    if (!builder) {
        return NULL;
    }

    Statement* stmt = createContinueStatement(location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建continue语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

Statement* astBuilderCreateSwitchStmt(ASTBuilder* builder, Expression* condition,
                                       Vector* cases, SourceLocation location) {
    if (!builder || !condition || !cases) {
        return NULL;
    }

    Statement* stmt = createSwitchStatement(condition, cases, location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建switch语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

Statement* astBuilderCreateCaseStmt(ASTBuilder* builder, CaseKind kind,
                                     Expression* value, Statement* statement,
                                     SourceLocation location) {
    if (!builder || !statement) {
        return NULL;
    }

    if (kind == CASE_LABEL && !value) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "case标签需要值表达式",
            location,
            NULL,
            0);
        return NULL;
    }

    Statement* stmt = createCaseStatement(kind, value, statement, location);
    if (!stmt) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建case语句",
            location,
            NULL,
            0);
        return NULL;
    }

    return stmt;
}

// ==================== 表达式构建函数 ====================

Expression* astBuilderCreateLiteralExpr(ASTBuilder* builder, Token* literal,
                                         SourceLocation location) {
    if (!builder) {
        return NULL;
    }

    Expression* expr = createLiteralExpr(literal, location);
    if (!expr) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建字面量表达式",
            location,
            NULL,
            0);
        return NULL;
    }

    return expr;
}

Expression* astBuilderCreateIdentifierExpr(ASTBuilder* builder, const char* name,
                                            SourceLocation location) {
    if (!builder || !name) {
        return NULL;
    }

    Expression* expr = createIdentifierExpr(name, location);
    if (!expr) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建标识符表达式",
            location,
            name,
            0);
        return NULL;
    }

    return expr;
}

Expression* astBuilderCreateBinaryOpExpr(ASTBuilder* builder, BinaryOperator op,
                                          Expression* left, Expression* right,
                                          SourceLocation location) {
    if (!builder || !left || !right) {
        return NULL;
    }

    Expression* expr = createBinaryOperatorExpr(op, left, right, location);
    if (!expr) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建二元运算符表达式",
            location,
            binaryOperatorToString(op),
            0);
        return NULL;
    }

    return expr;
}

Expression* astBuilderCreateUnaryOpExpr(ASTBuilder* builder, UnaryOperator op,
                                         Expression* operand, bool isPrefix,
                                         SourceLocation location) {
    if (!builder || !operand) {
        return NULL;
    }

    Expression* expr = createUnaryOperatorExpr(op, operand, isPrefix, location);
    if (!expr) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建一元运算符表达式",
            location,
            unaryOperatorToString(op),
            0);
        return NULL;
    }

    return expr;
}

Expression* astBuilderCreateAssignmentExpr(ASTBuilder* builder, AssignmentKind kind,
                                            Expression* left, Expression* right,
                                            SourceLocation location) {
    if (!builder || !left || !right) {
        return NULL;
    }

    Expression* expr = createAssignmentExpr(kind, left, right, location);
    if (!expr) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建赋值表达式",
            location,
            assignmentKindToString(kind),
            0);
        return NULL;
    }

    return expr;
}

Expression* astBuilderCreateTernaryExpr(ASTBuilder* builder, Expression* condition,
                                         Expression* thenExpr, Expression* elseExpr,
                                         SourceLocation location) {
    if (!builder || !condition || !thenExpr || !elseExpr) {
        return NULL;
    }

    Expression* expr = createTernaryExpr(condition, thenExpr, elseExpr, location);
    if (!expr) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建三元表达式",
            location,
            NULL,
            0);
        return NULL;
    }

    return expr;
}

Expression* astBuilderCreateFunctionCallExpr(ASTBuilder* builder, Expression* callee,
                                              Vector* arguments, SourceLocation location) {
    if (!builder || !callee) {
        return NULL;
    }

    Expression* expr = createFunctionCallExpr(callee, arguments, location);
    if (!expr) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建函数调用表达式",
            location,
            NULL,
            0);
        return NULL;
    }

    return expr;
}

Expression* astBuilderCreateArraySubscriptExpr(ASTBuilder* builder, Expression* array,
                                                Expression* index, SourceLocation location) {
    if (!builder || !array || !index) {
        return NULL;
    }

    Expression* expr = createArraySubscriptExpr(array, index, location);
    if (!expr) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建数组下标表达式",
            location,
            NULL,
            0);
        return NULL;
    }

    return expr;
}

Expression* astBuilderCreateMemberAccessExpr(ASTBuilder* builder, Expression* baseExpr,
                                              const char* memberName, bool isArrow,
                                              SourceLocation location) {
    if (!builder || !baseExpr || !memberName) {
        return NULL;
    }

    Expression* expr = createMemberAccessExpr(baseExpr, memberName, isArrow, location);
    if (!expr) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建成员访问表达式",
            location,
            memberName,
            0);
        return NULL;
    }

    return expr;
}

Expression* astBuilderCreateCastExpr(ASTBuilder* builder, TypeSpecifier* targetType,
                                      Expression* operand, SourceLocation location) {
    if (!builder || !targetType || !operand) {
        return NULL;
    }

    Expression* expr = createCastExpr(targetType, operand, location);
    if (!expr) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建类型转换表达式",
            location,
            NULL,
            0);
        return NULL;
    }

    return expr;
}

// ==================== 类型说明符构建函数 ====================

TypeSpecifier* astBuilderCreateBasicType(ASTBuilder* builder, BasicTypeKind kind,
                                           SourceLocation location) {
    if (!builder) {
        return NULL;
    }

    TypeSpecifier* type = createBasicTypeSpecifier(kind, location);
    if (!type) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建基础类型",
            location,
            basicTypeKindToString(kind),
            0);
        return NULL;
    }

    return type;
}

TypeSpecifier* astBuilderCreatePointerType(ASTBuilder* builder, TypeSpecifier* baseType,
                                            SourceLocation location) {
    if (!builder || !baseType) {
        return NULL;
    }

    TypeSpecifier* type = createPointerTypeSpecifier(baseType, location);
    if (!type) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建指针类型",
            location,
            NULL,
            0);
        return NULL;
    }

    return type;
}

TypeSpecifier* astBuilderCreateArrayType(ASTBuilder* builder, TypeSpecifier* elementType,
                                          Expression* size, SourceLocation location) {
    if (!builder || !elementType) {
        return NULL;
    }

    TypeSpecifier* type = createArrayTypeSpecifier(elementType, size, location);
    if (!type) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建数组类型",
            location,
            NULL,
            0);
        return NULL;
    }

    return type;
}

TypeSpecifier* astBuilderCreateFunctionType(ASTBuilder* builder, TypeSpecifier* returnType,
                                             Vector* parameterTypes, bool isVariadic,
                                             SourceLocation location) {
    if (!builder || !returnType) {
        return NULL;
    }

    TypeSpecifier* type = createFunctionTypeSpecifier(returnType, parameterTypes, isVariadic, location);
    if (!type) {
        diagnosticEngineEmitDiagnostic(builder->diagnostics,
            DIAGNOSTIC_ERROR,
            "无法创建函数类型",
            location,
            NULL,
            0);
        return NULL;
    }

    return type;
}

// ==================== 复合语句操作 ====================

bool astBuilderAddStmtToCompound(Statement* compound, Statement* stmt) {
    if (!compound || !stmt) {
        return false;
    }

    if (compound->base.nodeType != AST_NODE_COMPOUND_STATEMENT) {
        return false;
    }

    CompoundStatement* comp = (CompoundStatement*)compound;
    if (!comp->statements) {
        return false;
    }

    vectorPushBack(comp->statements, &stmt);
    stmt->base.parent = (ASTNode*)compound;

    return true;
}

bool astBuilderAddDeclToCompound(Statement* compound, Declaration* decl) {
    if (!compound || !decl) {
        return false;
    }

    if (compound->base.nodeType != AST_NODE_COMPOUND_STATEMENT) {
        return false;
    }

    CompoundStatement* comp = (CompoundStatement*)compound;
    if (!comp->declarations) {
        return false;
    }

    vectorPushBack(comp->declarations, &decl);
    decl->base.parent = (ASTNode*)compound;

    return true;
}

// ==================== 验证和诊断 ====================

bool astBuilderValidateExpr(ASTBuilder* builder, Expression* expr) {
    if (!builder || !expr) {
        return false;
    }

    // TODO: 实现表达式验证逻辑
    // - 检查类型一致性
    // - 检查操作符有效性
    // - 检查左值要求
    // 等等

    return true;
}

bool astBuilderValidateStmt(ASTBuilder* builder, Statement* stmt) {
    if (!builder || !stmt) {
        return false;
    }

    // TODO: 实现语句验证逻辑
    // - 检查控制流合法性
    // - 检查break/continue位置
    // - 检查return语句
    // 等等

    return true;
}

// ==================== 统计信息 ====================

size_t astBuilderGetNodeCount(ASTBuilder* builder) {
    if (!builder || !builder->root) {
        return 0;
    }

    // TODO: 实现节点计数功能
    // 可以使用访问者模式遍历AST并计数

    return 0;
}
