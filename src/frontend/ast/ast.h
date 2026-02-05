#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stddef.h>
#include "../../common/diagnostics/source_location.h"
#include "../../common/containers/vector.h"

// 前向声明
typedef struct Type Type;
typedef struct Symbol Symbol;
typedef struct ASTVisitor ASTVisitor;

// ==================== AST节点类型枚举 ====================

/**
 * @brief AST节点类型枚举
 */
typedef enum {
    // 基础类型
    AST_NODE_EXPRESSION,
    AST_NODE_STATEMENT,
    AST_NODE_DECLARATION,
    AST_NODE_TYPE_SPECIFIER,
    AST_NODE_TRANSLATION_UNIT,

    // 表达式子类型
    AST_NODE_LITERAL_EXPR,
    AST_NODE_IDENTIFIER_EXPR,
    AST_NODE_BINARY_OPERATOR_EXPR,
    AST_NODE_UNARY_OPERATOR_EXPR,
    AST_NODE_ASSIGNMENT_EXPR,
    AST_NODE_TERNARY_EXPR,
    AST_NODE_FUNCTION_CALL_EXPR,
    AST_NODE_ARRAY_SUBSCRIPT_EXPR,
    AST_NODE_MEMBER_ACCESS_EXPR,
    AST_NODE_CAST_EXPR,

    // 语句子类型
    AST_NODE_EXPRESSION_STATEMENT,
    AST_NODE_COMPOUND_STATEMENT,
    AST_NODE_IF_STATEMENT,
    AST_NODE_WHILE_STATEMENT,
    AST_NODE_DO_WHILE_STATEMENT,
    AST_NODE_FOR_STATEMENT,
    AST_NODE_RETURN_STATEMENT,
    AST_NODE_BREAK_STATEMENT,
    AST_NODE_CONTINUE_STATEMENT,
    AST_NODE_SWITCH_STATEMENT,
    AST_NODE_CASE_STATEMENT,
    AST_NODE_LABELED_STATEMENT,
    AST_NODE_GOTO_STATEMENT,

    // 声明子类型
    AST_NODE_VARIABLE_DECLARATION,
    AST_NODE_FUNCTION_DECLARATION,
    AST_NODE_STRUCT_DECLARATION,
    AST_NODE_UNION_DECLARATION,
    AST_NODE_ENUM_DECLARATION,
    AST_NODE_TYPEDEF_DECLARATION,

    // 类型说明符子类型
    AST_NODE_BASIC_TYPE_SPECIFIER,
    AST_NODE_POINTER_TYPE_SPECIFIER,
    AST_NODE_ARRAY_TYPE_SPECIFIER,
    AST_NODE_FUNCTION_TYPE_SPECIFIER,
    AST_NODE_STRUCT_TYPE_SPECIFIER,
    AST_NODE_UNION_TYPE_SPECIFIER,
    AST_NODE_ENUM_TYPE_SPECIFIER,
    AST_NODE_TYPEDEF_NAME_SPECIFIER
} ASTNodeType;

// ==================== 基础AST节点结构 ====================

/**
 * @brief 基础AST节点结构
 *
 * 所有AST节点都继承自此结构，使用C语言的"继承"模式：
 * - 子节点的第一个成员是ASTNode
 * - 通过函数指针表实现虚函数
 */
typedef struct ASTNode {
    SourceLocation location;       // 源代码位置
    struct ASTNode* parent;        // 父节点指针
    ASTNodeType nodeType;          // 节点类型

    // 虚函数表（函数指针）
    void (*accept)(struct ASTNode* self, ASTVisitor* visitor);
    void (*destroy)(struct ASTNode* self);
} ASTNode;

// ==================== 表达式基础结构 ====================

/**
 * @brief 表达式子类型枚举
 */
typedef enum {
    EXPR_LITERAL,
    EXPR_IDENTIFIER,
    EXPR_BINARY_OPERATOR,
    EXPR_UNARY_OPERATOR,
    EXPR_ASSIGNMENT,
    EXPR_TERNARY,
    EXPR_FUNCTION_CALL,
    EXPR_ARRAY_SUBSCRIPT,
    EXPR_MEMBER_ACCESS,
    EXPR_CAST
} ExpressionKind;

/**
 * @brief 表达式基础结构
 *
 * 所有表达式节点都继承自此结构
 */
typedef struct Expression {
    ASTNode base;
    Type* type;                    // 类型（由语义分析器设置）
    ExpressionKind exprKind;       // 表达式子类型
    bool isLvalue;                 // 是否为左值
    bool isConstant;               // 是否为常量表达式
} Expression;

// ==================== 语句基础结构 ====================

/**
 * @brief 语句子类型枚举
 */
typedef enum {
    STMT_EXPRESSION,
    STMT_COMPOUND,
    STMT_IF,
    STMT_WHILE,
    STMT_DO_WHILE,
    STMT_FOR,
    STMT_RETURN,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_SWITCH,
    STMT_CASE,
    STMT_LABELED,
    STMT_GOTO
} StatementKind;

/**
 * @brief 语句基础结构
 *
 * 所有语句节点都继承自此结构
 */
typedef struct Statement {
    ASTNode base;
    StatementKind stmtKind;        // 语句子类型
} Statement;

// ==================== 声明基础结构 ====================

/**
 * @brief 存储类说明符
 */
typedef enum {
    STORAGE_CLASS_AUTO,
    STORAGE_CLASS_STATIC,
    STORAGE_CLASS_EXTERN,
    STORAGE_CLASS_REGISTER,
    STORAGE_CLASS_THREAD_LOCAL,
    STORAGE_CLASS_NONE             // 默认
} StorageClassSpecifier;

/**
 * @brief 声明子类型枚举
 */
typedef enum {
    DECL_VARIABLE,
    DECL_FUNCTION,
    DECL_STRUCT,
    DECL_UNION,
    DECL_ENUM,
    DECL_TYPEDEF
} DeclarationKind;

/**
 * @brief 声明基础结构
 *
 * 所有声明节点都继承自此结构
 */
typedef struct Declaration {
    ASTNode base;
    char* name;                    // 声明的名称
    DeclarationKind declKind;      // 声明子类型
    Symbol* symbol;                // 符号表条目（由语义分析器设置）
    StorageClassSpecifier storageClass;  // 存储类
} Declaration;

// ==================== 类型说明符基础结构 ====================

/**
 * @brief 类型说明符子类型枚举
 */
typedef enum {
    TYPE_SPECIFIER_BASIC,
    TYPE_SPECIFIER_POINTER,
    TYPE_SPECIFIER_ARRAY,
    TYPE_SPECIFIER_FUNCTION,
    TYPE_SPECIFIER_STRUCT,
    TYPE_SPECIFIER_UNION,
    TYPE_SPECIFIER_ENUM,
    TYPE_SPECIFIER_TYPEDEF_NAME
} TypeSpecifierKind;

/**
 * @brief 类型说明符基础结构
 *
 * 所有类型说明符节点都继承自此结构
 */
typedef struct TypeSpecifier {
    ASTNode base;
    TypeSpecifierKind typeKind;    // 类型说明符子类型
    bool isConst;                  // const 限定符
    bool isVolatile;               // volatile 限定符
} TypeSpecifier;

// ==================== 翻译单元（根节点）====================

/**
 * @brief 翻译单元结构
 *
 * 这是AST的根节点，代表整个编译单元
 */
typedef struct TranslationUnit {
    ASTNode base;
    Vector* declarations;          // Declaration*
} TranslationUnit;

// ==================== 构造函数和析构函数 ====================

/**
 * @brief 创建AST节点
 * @param type 节点类型
 * @param location 源位置
 * @return 新创建的AST节点，失败返回NULL
 */
ASTNode* createASTNode(ASTNodeType type, SourceLocation location);

/**
 * @brief 销毁AST节点
 * @param node 要销毁的节点
 */
void destroyASTNode(ASTNode* node);

/**
 * @brief 创建翻译单元
 * @return 新创建的翻译单元，失败返回NULL
 */
TranslationUnit* createTranslationUnit(void);

/**
 * @brief 销毁翻译单元
 * @param unit 要销毁的翻译单元
 */
void destroyTranslationUnit(TranslationUnit* unit);

// ==================== 类型检查辅助函数 ====================

/**
 * @brief 检查节点是否为表达式
 */
static inline bool astNodeIsExpression(const ASTNode* node) {
    return node && (node->nodeType == AST_NODE_EXPRESSION ||
                    node->nodeType >= AST_NODE_LITERAL_EXPR &&
                    node->nodeType <= AST_NODE_CAST_EXPR);
}

/**
 * @brief 检查节点是否为语句
 */
static inline bool astNodeIsStatement(const ASTNode* node) {
    return node && (node->nodeType == AST_NODE_STATEMENT ||
                    node->nodeType >= AST_NODE_EXPRESSION_STATEMENT &&
                    node->nodeType <= AST_NODE_GOTO_STATEMENT);
}

/**
 * @brief 检查节点是否为声明
 */
static inline bool astNodeIsDeclaration(const ASTNode* node) {
    return node && (node->nodeType == AST_NODE_DECLARATION ||
                    node->nodeType >= AST_NODE_VARIABLE_DECLARATION &&
                    node->nodeType <= AST_NODE_TYPEDEF_DECLARATION);
}

/**
 * @brief 检查节点是否为类型说明符
 */
static inline bool astNodeIsTypeSpecifier(const ASTNode* node) {
    return node && (node->nodeType == AST_NODE_TYPE_SPECIFIER ||
                    node->nodeType >= AST_NODE_BASIC_TYPE_SPECIFIER &&
                    node->nodeType <= AST_NODE_TYPEDEF_NAME_SPECIFIER);
}

/**
 * @brief 获取AST节点类型名称
 * @param type 节点类型
 * @return 类型名称字符串
 */
const char* astNodeTypeName(ASTNodeType type);

// ==================== 树遍历辅助函数 ====================

/**
 * @brief 设置节点的父节点
 * @param node 子节点
 * @param parent 父节点
 */
void astNodeSetParent(ASTNode* node, ASTNode* parent);

/**
 * @brief 获取节点的源位置
 * @param node AST节点
 * @return 源位置
 */
SourceLocation astNodeGetLocation(const ASTNode* node);

// ==================== 类型转换宏 ====================

// 安全向下转换宏
#define AST_AS_NODE(ptr) ((ASTNode*)(ptr))
#define AST_AS_EXPRESSION(ptr) ((Expression*)(ptr))
#define AST_AS_STATEMENT(ptr) ((Statement*)(ptr))
#define AST_AS_DECLARATION(ptr) ((Declaration*)(ptr))
#define AST_AS_TYPE_SPECIFIER(ptr) ((TypeSpecifier*)(ptr))
#define AST_AS_TRANSLATION_UNIT(ptr) ((TranslationUnit*)(ptr))

// 销毁宏 - 对所有 AST 节点类型都有效
#define DESTROY_AST_NODE(node) \
    do { if ((node) != NULL) { ASTNode* _node = (ASTNode*)(node); _node->destroy(_node); (node) = NULL; } } while(0)

// 类型检查宏
#define AST_IS_LITERAL_EXPR(expr) \
    ((expr) != NULL && AST_AS_EXPRESSION(expr)->base.nodeType == AST_NODE_LITERAL_EXPR)
#define AST_IS_IDENTIFIER_EXPR(expr) \
    ((expr) != NULL && AST_AS_EXPRESSION(expr)->base.nodeType == AST_NODE_IDENTIFIER_EXPR)

#endif // AST_H
