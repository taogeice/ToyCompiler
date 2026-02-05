#ifndef AST_BUILDER_H
#define AST_BUILDER_H

#include "ast.h"
#include "ast_nodes.h"
#include "../../common/diagnostics/diagnostic_engine.h"

// 前向声明 - SymbolTable 尚未实现
typedef struct SymbolTable SymbolTable;

// ==================== AST构建器结构 ====================

/**
 * @brief AST构建器
 *
 * 提供高级API用于构建AST，简化Parser与AST模块的集成。
 *
 * 主要功能：
 * - 管理翻译单元（根节点）
 * - 提供类型安全的节点创建函数
 * - 自动处理父子关系
 * - 集成错误诊断
 * - 管理节点内存池（可选）
 * - 维护作用域栈
 *
 * 使用示例：
 * @code
 * // 创建构建器
 * ASTBuilder* builder = createASTBuilder(diagnostics);
 *
 * // 创建main函数
 * TypeSpecifier* intType = createBasicTypeSpecifier(BASIC_TYPE_INT, location);
 * CompoundStatement* body = createCompoundStatement(location);
 *
 * Declaration* mainFunc = astBuilderAddFunctionDecl(builder, "main",
 *     intType, NULL, body, location);
 *
 * // 添加return语句
 * Expression* zero = createLiteralExpr(createIntegerToken("0", location), location);
 * Statement* returnStmt = astBuilderCreateReturnStmt(builder, zero, location);
 * astBuilderAddStmtToCompound(body, returnStmt);
 *
 * // 获取最终AST
 * TranslationUnit* root = astBuilderGetRoot(builder);
 *
 * // 清理
 * destroyASTBuilder(builder);
 * @endcode
 */
typedef struct {
    TranslationUnit* root;          // AST根节点（翻译单元）
    DiagnosticEngine* diagnostics;  // 诊断引擎（用于错误报告）
    Vector* nodePool;               // 节点内存池（用于内存管理）
    Vector* scopeStack;             // 作用域栈（用于符号表管理）
} ASTBuilder;

// ==================== 构造函数和析构函数 ====================

/**
 * @brief 创建AST构建器
 * @param diagnostics 诊断引擎（必需）
 * @return 新创建的构建器，失败返回NULL
 *
 * 创建一个新的AST构建器实例，初始化根节点和辅助结构。
 */
ASTBuilder* createASTBuilder(DiagnosticEngine* diagnostics);

/**
 * @brief 销毁AST构建器
 * @param builder 要销毁的构建器
 *
 * 销毁构建器及其管理的所有AST节点。
 * 注意：这会递归销毁整个AST树。
 */
void destroyASTBuilder(ASTBuilder* builder);

// ==================== 访问器函数 ====================

/**
 * @brief 获取构建器的根节点
 * @param builder 构建器
 * @return 翻译单元（AST根节点）
 */
TranslationUnit* astBuilderGetRoot(ASTBuilder* builder);

/**
 * @brief 获取构建器的诊断引擎
 * @param builder 构建器
 * @return 诊断引擎
 */
DiagnosticEngine* astBuilderGetDiagnostics(ASTBuilder* builder);

// ==================== 声明构建函数 ====================

/**
 * @brief 添加变量声明
 * @param builder 构建器
 * @param name 变量名称
 * @param type 变量类型
 * @param initializer 初始化表达式（可以为NULL）
 * @param location 源位置
 * @return 新创建的变量声明，失败返回NULL
 */
Declaration* astBuilderAddVariableDecl(ASTBuilder* builder, const char* name,
                                        TypeSpecifier* type, Expression* initializer,
                                        SourceLocation location);

/**
 * @brief 添加函数声明
 * @param builder 构建器
 * @param name 函数名称
 * @param returnType 返回类型
 * @param parameters 参数列表（VariableDeclaration*向量，可以为NULL）
 * @param body 函数体（NULL表示函数原型）
 * @param location 源位置
 * @return 新创建的函数声明，失败返回NULL
 */
Declaration* astBuilderAddFunctionDecl(ASTBuilder* builder, const char* name,
                                        TypeSpecifier* returnType, Vector* parameters,
                                        CompoundStatement* body, SourceLocation location);

/**
 * @brief 添加结构体声明
 * @param builder 构建器
 * @param name 结构体名称（NULL表示匿名结构体）
 * @param members 成员列表（VariableDeclaration*向量）
 * @param location 源位置
 * @return 新创建的结构体声明，失败返回NULL
 */
Declaration* astBuilderAddStructDecl(ASTBuilder* builder, const char* name,
                                      Vector* members, SourceLocation location);

/**
 * @brief 添加联合体声明
 * @param builder 构建器
 * @param name 联合体名称（NULL表示匿名联合体）
 * @param members 成员列表（VariableDeclaration*向量）
 * @param location 源位置
 * @return 新创建的联合体声明，失败返回NULL
 */
Declaration* astBuilderAddUnionDecl(ASTBuilder* builder, const char* name,
                                     Vector* members, SourceLocation location);

/**
 * @brief 添加枚举声明
 * @param builder 构建器
 * @param name 枚举名称（NULL表示匿名枚举）
 * @param constants 常量列表（EnumConstant*向量）
 * @param location 源位置
 * @return 新创建的枚举声明，失败返回NULL
 */
Declaration* astBuilderAddEnumDecl(ASTBuilder* builder, const char* name,
                                    Vector* constants, SourceLocation location);

/**
 * @brief 添加typedef声明
 * @param builder 构建器
 * @param name 类型名称
 * @param aliasedType 被别名化的类型
 * @param location 源位置
 * @return 新创建的typedef声明，失败返回NULL
 */
Declaration* astBuilderAddTypedefDecl(ASTBuilder* builder, const char* name,
                                       TypeSpecifier* aliasedType, SourceLocation location);

// ==================== 语句构建函数 ====================

/**
 * @brief 创建表达式语句
 * @param builder 构建器
 * @param expression 表达式
 * @param location 源位置
 * @return 新创建的表达式语句，失败返回NULL
 */
Statement* astBuilderCreateExprStmt(ASTBuilder* builder, Expression* expression,
                                      SourceLocation location);

/**
 * @brief 创建复合语句（块）
 * @param builder 构建器
 * @param location 源位置
 * @return 新创建的复合语句，失败返回NULL
 */
Statement* astBuilderCreateCompoundStmt(ASTBuilder* builder, SourceLocation location);

/**
 * @brief 创建if语句
 * @param builder 构建器
 * @param condition 条件表达式
 * @param thenStmt then分支语句
 * @param elseStmt else分支语句（可以为NULL）
 * @param location 源位置
 * @return 新创建的if语句，失败返回NULL
 */
Statement* astBuilderCreateIfStmt(ASTBuilder* builder, Expression* condition,
                                   Statement* thenStmt, Statement* elseStmt,
                                   SourceLocation location);

/**
 * @brief 创建while循环语句
 * @param builder 构建器
 * @param condition 条件表达式
 * @param body 循环体
 * @param location 源位置
 * @return 新创建的while语句，失败返回NULL
 */
Statement* astBuilderCreateWhileStmt(ASTBuilder* builder, Expression* condition,
                                      Statement* body, SourceLocation location);

/**
 * @brief 创建do-while循环语句
 * @param builder 构建器
 * @param body 循环体
 * @param condition 条件表达式
 * @param location 源位置
 * @return 新创建的do-while语句，失败返回NULL
 */
Statement* astBuilderCreateDoWhileStmt(ASTBuilder* builder, Statement* body,
                                        Expression* condition, SourceLocation location);

/**
 * @brief 创建for循环语句
 * @param builder 构建器
 * @param init 初始化表达式（可以为NULL）
 * @param condition 条件表达式（可以为NULL）
 * @param increment 增量表达式（可以为NULL）
 * @param body 循环体
 * @param location 源位置
 * @return 新创建的for语句，失败返回NULL
 */
Statement* astBuilderCreateForStmt(ASTBuilder* builder, Expression* init,
                                    Expression* condition, Expression* increment,
                                    Statement* body, SourceLocation location);

/**
 * @brief 创建return语句
 * @param builder 构建器
 * @param returnValue 返回值表达式（可以为NULL）
 * @param location 源位置
 * @return 新创建的return语句，失败返回NULL
 */
Statement* astBuilderCreateReturnStmt(ASTBuilder* builder, Expression* returnValue,
                                       SourceLocation location);

/**
 * @brief 创建break语句
 * @param builder 构建器
 * @param location 源位置
 * @return 新创建的break语句，失败返回NULL
 */
Statement* astBuilderCreateBreakStmt(ASTBuilder* builder, SourceLocation location);

/**
 * @brief 创建continue语句
 * @param builder 构建器
 * @param location 源位置
 * @return 新创建的continue语句，失败返回NULL
 */
Statement* astBuilderCreateContinueStmt(ASTBuilder* builder, SourceLocation location);

/**
 * @brief 创建switch语句
 * @param builder 构建器
 * @param condition 控制表达式
 * @param cases case语句向量（CaseStatement*）
 * @param location 源位置
 * @return 新创建的switch语句，失败返回NULL
 */
Statement* astBuilderCreateSwitchStmt(ASTBuilder* builder, Expression* condition,
                                       Vector* cases, SourceLocation location);

/**
 * @brief 创建case/default语句
 * @param builder 构建器
 * @param kind CASE_LABEL或DEFAULT_LABEL
 * @param value case值表达式（DEFAULT_LABEL时为NULL）
 * @param statement 关联的语句
 * @param location 源位置
 * @return 新创建的case语句，失败返回NULL
 */
Statement* astBuilderCreateCaseStmt(ASTBuilder* builder, CaseKind kind,
                                     Expression* value, Statement* statement,
                                     SourceLocation location);

// ==================== 表达式构建函数 ====================

/**
 * @brief 创建字面量表达式
 * @param builder 构建器
 * @param literal 字面量token
 * @param location 源位置
 * @return 新创建的字面量表达式，失败返回NULL
 */
Expression* astBuilderCreateLiteralExpr(ASTBuilder* builder, Token* literal,
                                         SourceLocation location);

/**
 * @brief 创建标识符表达式
 * @param builder 构建器
 * @param name 标识符名称
 * @param location 源位置
 * @return 新创建的标识符表达式，失败返回NULL
 */
Expression* astBuilderCreateIdentifierExpr(ASTBuilder* builder, const char* name,
                                            SourceLocation location);

/**
 * @brief 创建二元运算符表达式
 * @param builder 构建器
 * @param op 运算符类型
 * @param left 左操作数
 * @param right 右操作数
 * @param location 源位置
 * @return 新创建的二元运算符表达式，失败返回NULL
 */
Expression* astBuilderCreateBinaryOpExpr(ASTBuilder* builder, BinaryOperator op,
                                          Expression* left, Expression* right,
                                          SourceLocation location);

/**
 * @brief 创建一元运算符表达式
 * @param builder 构建器
 * @param op 运算符类型
 * @param operand 操作数
 * @param isPrefix 是否为前缀运算符
 * @param location 源位置
 * @return 新创建的一元运算符表达式，失败返回NULL
 */
Expression* astBuilderCreateUnaryOpExpr(ASTBuilder* builder, UnaryOperator op,
                                         Expression* operand, bool isPrefix,
                                         SourceLocation location);

/**
 * @brief 创建赋值表达式
 * @param builder 构建器
 * @param kind 赋值运算符类型
 * @param left 左值（必须为左值表达式）
 * @param right 右值
 * @param location 源位置
 * @return 新创建的赋值表达式，失败返回NULL
 */
Expression* astBuilderCreateAssignmentExpr(ASTBuilder* builder, AssignmentKind kind,
                                            Expression* left, Expression* right,
                                            SourceLocation location);

/**
 * @brief 创建三元条件运算符表达式
 * @param builder 构建器
 * @param condition 条件表达式
 * @param thenExpr then表达式
 * @param elseExpr else表达式
 * @param location 源位置
 * @return 新创建的三元表达式，失败返回NULL
 */
Expression* astBuilderCreateTernaryExpr(ASTBuilder* builder, Expression* condition,
                                         Expression* thenExpr, Expression* elseExpr,
                                         SourceLocation location);

/**
 * @brief 创建函数调用表达式
 * @param builder 构建器
 * @param callee 被调用函数表达式
 * @param arguments 参数向量（Expression*）
 * @param location 源位置
 * @return 新创建的函数调用表达式，失败返回NULL
 */
Expression* astBuilderCreateFunctionCallExpr(ASTBuilder* builder, Expression* callee,
                                              Vector* arguments, SourceLocation location);

/**
 * @brief 创建数组下标表达式
 * @param builder 构建器
 * @param array 数组表达式
 * @param index 索引表达式
 * @param location 源位置
 * @return 新创建的数组下标表达式，失败返回NULL
 */
Expression* astBuilderCreateArraySubscriptExpr(ASTBuilder* builder, Expression* array,
                                                Expression* index, SourceLocation location);

/**
 * @brief 创建成员访问表达式
 * @param builder 构建器
 * @param baseExpr 基础表达式
 * @param memberName 成员名称
 * @param isArrow true使用->，false使用.
 * @param location 源位置
 * @return 新创建的成员访问表达式，失败返回NULL
 */
Expression* astBuilderCreateMemberAccessExpr(ASTBuilder* builder, Expression* baseExpr,
                                              const char* memberName, bool isArrow,
                                              SourceLocation location);

/**
 * @brief 创建类型转换表达式
 * @param builder 构建器
 * @param targetType 目标类型
 * @param operand 操作数
 * @param location 源位置
 * @return 新创建的类型转换表达式，失败返回NULL
 */
Expression* astBuilderCreateCastExpr(ASTBuilder* builder, TypeSpecifier* targetType,
                                      Expression* operand, SourceLocation location);

// ==================== 类型说明符构建函数 ====================

/**
 * @brief 创建基础类型
 * @param builder 构建器
 * @param kind 基础类型种类
 * @param location 源位置
 * @return 新创建的基础类型，失败返回NULL
 */
TypeSpecifier* astBuilderCreateBasicType(ASTBuilder* builder, BasicTypeKind kind,
                                           SourceLocation location);

/**
 * @brief 创建指针类型
 * @param builder 构建器
 * @param baseType 基础类型
 * @param location 源位置
 * @return 新创建的指针类型，失败返回NULL
 */
TypeSpecifier* astBuilderCreatePointerType(ASTBuilder* builder, TypeSpecifier* baseType,
                                            SourceLocation location);

/**
 * @brief 创建数组类型
 * @param builder 构建器
 * @param elementType 元素类型
 * @param size 数组大小表达式（NULL表示未指定大小）
 * @param location 源位置
 * @return 新创建的数组类型，失败返回NULL
 */
TypeSpecifier* astBuilderCreateArrayType(ASTBuilder* builder, TypeSpecifier* elementType,
                                          Expression* size, SourceLocation location);

/**
 * @brief 创建函数类型
 * @param builder 构建器
 * @param returnType 返回类型
 * @param parameterTypes 参数类型向量（TypeSpecifier*）
 * @param isVariadic 是否为可变参数函数
 * @param location 源位置
 * @return 新创建的函数类型，失败返回NULL
 */
TypeSpecifier* astBuilderCreateFunctionType(ASTBuilder* builder, TypeSpecifier* returnType,
                                             Vector* parameterTypes, bool isVariadic,
                                             SourceLocation location);

// ==================== 复合语句操作 ====================

/**
 * @brief 向复合语句添加语句
 * @param compound 复合语句
 * @param stmt 要添加的语句
 * @return 成功返回true，失败返回false
 *
 * 自动设置父子关系。
 */
bool astBuilderAddStmtToCompound(Statement* compound, Statement* stmt);

/**
 * @brief 向复合语句添加声明
 * @param compound 复合语句
 * @param decl 要添加的声明
 * @return 成功返回true，失败返回false
 *
 * 自动设置父子关系。
 */
bool astBuilderAddDeclToCompound(Statement* compound, Declaration* decl);

// ==================== 验证和诊断 ====================

/**
 * @brief 验证表达式
 * @param builder 构建器
 * @param expr 要验证的表达式
 * @return 有效返回true，无效返回false
 *
 * 验证内容包括：
 * - 类型一致性
 * - 操作符有效性
 * - 左值要求
 * - 常量表达式要求
 */
bool astBuilderValidateExpr(ASTBuilder* builder, Expression* expr);

/**
 * @brief 验证语句
 * @param builder 构建器
 * @param stmt 要验证的语句
 * @return 有效返回true，无效返回false
 *
 * 验证内容包括：
 * - 控制流合法性
 * - break/continue位置
 * - return语句完整性
 * - 未到达代码检测
 */
bool astBuilderValidateStmt(ASTBuilder* builder, Statement* stmt);

// ==================== 统计信息 ====================

/**
 * @brief 获取AST节点总数
 * @param builder 构建器
 * @return 节点总数
 */
size_t astBuilderGetNodeCount(ASTBuilder* builder);

#endif // AST_BUILDER_H
