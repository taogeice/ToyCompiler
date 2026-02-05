#ifndef AST_NODES_H
#define AST_NODES_H

#include "ast.h"
#include "../../frontend/lexer/token.h"

// ==================== 表达式节点类型 ====================

/**
 * @brief 二元运算符类型
 */
typedef enum {
    BINOP_ADD,                // +
    BINOP_SUB,                // -
    BINOP_MUL,                // *
    BINOP_DIV,                // /
    BINOP_MOD,                // %
    BINOP_EQ,                 // ==
    BINOP_NE,                 // !=
    BINOP_LT,                 // <
    BINOP_LE,                 // <=
    BINOP_GT,                 // >
    BINOP_GE,                 // >=
    BINOP_LOGICAL_AND,        // &&
    BINOP_LOGICAL_OR,         // ||
    BINOP_BITWISE_AND,        // &
    BINOP_BITWISE_OR,         // |
    BINOP_BITWISE_XOR,        // ^
    BINOP_LEFT_SHIFT,         // <<
    BINOP_RIGHT_SHIFT,        // >>
    BINOP_COMMA               // ,
} BinaryOperator;

/**
 * @brief 一元运算符类型
 */
typedef enum {
    UNOP_POSTFIX_INC,         // i++
    UNOP_POSTFIX_DEC,         // i--
    UNOP_PREFIX_INC,          // ++i
    UNOP_PREFIX_DEC,          // --i
    UNOP_PLUS,                // +
    UNOP_MINUS,               // -
    UNOP_BITWISE_NOT,         // ~
    UNOP_LOGICAL_NOT,         // !
    UNOP_DEREF,               // *
    UNOP_ADDRESS_OF,          // &
    UNOP_SIZEOF               // sizeof
} UnaryOperator;

/**
 * @brief 赋值运算符类型
 */
typedef enum {
    ASSIGN_SIMPLE,            // =
    ASSIGN_ADD,               // +=
    ASSIGN_SUB,               // -=
    ASSIGN_MUL,               // *=
    ASSIGN_DIV,               // /=
    ASSIGN_MOD,               // %=
    ASSIGN_SHL,               // <<=
    ASSIGN_SHR,               // >>=
    ASSIGN_AND,               // &=
    ASSIGN_OR,                // |=
    ASSIGN_XOR                // ^=
} AssignmentKind;

/**
 * @brief 字面量表达式
 */
typedef struct {
    Expression base;
    Token* literalToken;        // INTEGER_LITERAL, FLOAT_LITERAL, CHAR_LITERAL, STRING_LITERAL
} LiteralExpr;

/**
 * @brief 标识符表达式
 */
typedef struct {
    Expression base;
    char* name;                // 标识符名称
    Symbol* symbol;            // 符号表条目（由语义分析器解析）
} IdentifierExpr;

/**
 * @brief 二元运算符表达式
 */
typedef struct {
    Expression base;
    BinaryOperator op;
    Expression* left;
    Expression* right;
} BinaryOperatorExpr;

/**
 * @brief 一元运算符表达式
 */
typedef struct {
    Expression base;
    UnaryOperator op;
    Expression* operand;
    bool isPrefix;             // true for ++i, false for i++ (for increment/decrement)
} UnaryOperatorExpr;

/**
 * @brief 赋值表达式
 */
typedef struct {
    Expression base;
    AssignmentKind kind;
    Expression* left;          // 必须是左值
    Expression* right;
} AssignmentExpr;

/**
 * @brief 三元条件运算符表达式 (?:)
 */
typedef struct {
    Expression base;
    Expression* condition;
    Expression* thenExpr;
    Expression* elseExpr;
} TernaryExpr;

/**
 * @brief 函数调用表达式
 */
typedef struct {
    Expression base;
    Expression* callee;        // 函数标识符或成员访问表达式
    Vector* arguments;         // Expression*
} FunctionCallExpr;

/**
 * @brief 数组下标表达式
 */
typedef struct {
    Expression base;
    Expression* array;
    Expression* index;
} ArraySubscriptExpr;

/**
 * @brief 成员访问表达式
 */
typedef struct {
    Expression base;
    Expression* baseExpr;
    char* memberName;
    bool isArrow;              // true for ->, false for .
} MemberAccessExpr;

/**
 * @brief 类型转换表达式
 */
typedef struct {
    Expression base;
    TypeSpecifier* targetType;
    Expression* operand;
} CastExpr;

// ==================== 语句节点类型 ====================

/**
 * @brief 表达式语句
 */
typedef struct {
    Statement base;
    Expression* expression;
} ExpressionStatement;

/**
 * @brief 复合语句（块）
 */
typedef struct {
    Statement base;
    Vector* declarations;      // Declaration* (块内的局部声明)
    Vector* statements;        // Statement*
} CompoundStatement;

/**
 * @brief if 语句
 */
typedef struct {
    Statement base;
    Expression* condition;
    Statement* thenStmt;
    Statement* elseStmt;       // 可以为 NULL
} IfStatement;

/**
 * @brief while 循环语句
 */
typedef struct {
    Statement base;
    Expression* condition;
    Statement* body;
} WhileStatement;

/**
 * @brief do-while 循环语句
 */
typedef struct {
    Statement base;
    Statement* body;
    Expression* condition;
} DoWhileStatement;

/**
 * @brief for 循环语句
 */
typedef struct {
    Statement base;
    Expression* init;          // 初始化表达式（可以是声明）
    Expression* condition;     // 条件表达式（可以为 NULL）
    Expression* increment;     // 增量表达式（可以为 NULL）
    Statement* body;
} ForStatement;

/**
 * @brief return 语句
 */
typedef struct {
    Statement base;
    Expression* returnValue;   // 可以为 NULL
} ReturnStatement;

/**
 * @brief break 语句
 */
typedef struct {
    Statement base;
} BreakStatement;

/**
 * @brief continue 语句
 */
typedef struct {
    Statement base;
} ContinueStatement;

/**
 * @brief switch 语句
 */
typedef struct {
    Statement base;
    Expression* condition;
    Vector* cases;             // CaseStatement*
} SwitchStatement;

/**
 * @brief case/default 标签语句
 */
typedef enum {
    CASE_LABEL,
    DEFAULT_LABEL
} CaseKind;

typedef struct {
    Statement base;
    CaseKind kind;
    Expression* value;         // CASE_LABEL 时的常量值，DEFAULT_LABEL 时为 NULL
    Statement* statement;
} CaseStatement;

/**
 * @brief 标签语句（用于 goto）
 */
typedef struct {
    Statement base;
    char* labelName;
    Statement* statement;
} LabeledStatement;

/**
 * @brief goto 语句
 */
typedef struct {
    Statement base;
    char* labelName;
} GotoStatement;

// ==================== 声明节点类型 ====================

/**
 * @brief 变量声明
 */
typedef struct {
    Declaration base;
    TypeSpecifier* type;
    Expression* initializer;    // 可以为 NULL
    bool isConst;
    bool isVolatile;
} VariableDeclaration;

/**
 * @brief 函数声明
 */
typedef struct {
    Declaration base;
    TypeSpecifier* returnType;
    Vector* parameters;        // VariableDeclaration*
    CompoundStatement* body;    // NULL 表示函数原型
    bool isInline;
    bool isNoreturn;
} FunctionDeclaration;

/**
 * @brief 结构体声明
 */
typedef struct {
    Declaration base;
    Vector* members;           // VariableDeclaration*
    bool isPacked;             // 用于属性支持
} StructDeclaration;

/**
 * @brief 联合体声明
 */
typedef struct {
    Declaration base;
    Vector* members;           // VariableDeclaration*
} UnionDeclaration;

/**
 * @brief 枚举常量
 */
typedef struct {
    char* name;
    Expression* value;         // 可以为 NULL
} EnumConstant;

/**
 * @brief 枚举声明
 */
typedef struct {
    Declaration base;
    Vector* constants;         // EnumConstant*
    Type* underlyingType;      // 底层类型（通常是 int）
} EnumDeclaration;

/**
 * @brief typedef 声明
 */
typedef struct {
    Declaration base;
    TypeSpecifier* aliasedType;
} TypedefDeclaration;

// ==================== 类型说明符节点类型 ====================

/**
 * @brief 基础类型说明符
 */
typedef enum {
    BASIC_TYPE_VOID,
    BASIC_TYPE_CHAR,
    BASIC_TYPE_SHORT,
    BASIC_TYPE_INT,
    BASIC_TYPE_LONG,
    BASIC_TYPE_FLOAT,
    BASIC_TYPE_DOUBLE,
    BASIC_TYPE_SIGNED,
    BASIC_TYPE_UNSIGNED,
    BASIC_TYPE_BOOL,
    BASIC_TYPE_COMPLEX         // C99
} BasicTypeKind;

typedef struct {
    TypeSpecifier base;
    BasicTypeKind kind;
    bool isLong;               // 用于 long long
    bool isShort;
    bool isSigned;
    bool isUnsigned;
} BasicTypeSpecifier;

/**
 * @brief 指针类型说明符
 */
typedef struct {
    TypeSpecifier base;
    TypeSpecifier* baseType;
} PointerTypeSpecifier;

/**
 * @brief 数组类型说明符
 */
typedef struct {
    TypeSpecifier base;
    TypeSpecifier* elementType;
    Expression* size;          // NULL 表示未指定大小或 []
    bool isVariableLength;     // C99 VLA
} ArrayTypeSpecifier;

/**
 * @brief 函数类型说明符
 */
typedef struct {
    TypeSpecifier base;
    TypeSpecifier* returnType;
    Vector* parameterTypes;    // TypeSpecifier*
    bool isVariadic;
} FunctionTypeSpecifier;

/**
 * @brief 结构体类型说明符
 */
typedef struct {
    TypeSpecifier base;
    char* name;                // NULL 表示匿名结构体
    StructDeclaration* declaration;  // 可以为 NULL（前向声明）
} StructTypeSpecifier;

/**
 * @brief 联合体类型说明符
 */
typedef struct {
    TypeSpecifier base;
    char* name;                // NULL 表示匿名联合体
    UnionDeclaration* declaration;   // 可以为 NULL（前向声明）
} UnionTypeSpecifier;

/**
 * @brief 枚举类型说明符
 */
typedef struct {
    TypeSpecifier base;
    char* name;                // NULL 表示匿名枚举
    EnumDeclaration* declaration;    // 可以为 NULL（前向声明）
} EnumTypeSpecifier;

/**
 * @brief typedef 名称类型说明符
 */
typedef struct {
    TypeSpecifier base;
    char* typedefName;
} TypedefNameSpecifier;

// ==================== 工厂函数声明 ====================

// 表达式工厂函数
Expression* createLiteralExpr(Token* literal, SourceLocation location);
Expression* createIdentifierExpr(const char* name, SourceLocation location);
Expression* createBinaryOperatorExpr(BinaryOperator op, Expression* left, Expression* right, SourceLocation location);
Expression* createUnaryOperatorExpr(UnaryOperator op, Expression* operand, bool isPrefix, SourceLocation location);
Expression* createAssignmentExpr(AssignmentKind kind, Expression* left, Expression* right, SourceLocation location);
Expression* createTernaryExpr(Expression* condition, Expression* thenExpr, Expression* elseExpr, SourceLocation location);
Expression* createFunctionCallExpr(Expression* callee, Vector* arguments, SourceLocation location);
Expression* createArraySubscriptExpr(Expression* array, Expression* index, SourceLocation location);
Expression* createMemberAccessExpr(Expression* baseExpr, const char* memberName, bool isArrow, SourceLocation location);
Expression* createCastExpr(TypeSpecifier* targetType, Expression* operand, SourceLocation location);

// 语句工厂函数
Statement* createExpressionStatement(Expression* expression, SourceLocation location);
Statement* createCompoundStatement(SourceLocation location);
Statement* createIfStatement(Expression* condition, Statement* thenStmt, Statement* elseStmt, SourceLocation location);
Statement* createWhileStatement(Expression* condition, Statement* body, SourceLocation location);
Statement* createDoWhileStatement(Statement* body, Expression* condition, SourceLocation location);
Statement* createForStatement(Expression* init, Expression* condition, Expression* increment, Statement* body, SourceLocation location);
Statement* createReturnStatement(Expression* returnValue, SourceLocation location);
Statement* createBreakStatement(SourceLocation location);
Statement* createContinueStatement(SourceLocation location);
Statement* createSwitchStatement(Expression* condition, Vector* cases, SourceLocation location);
Statement* createCaseStatement(CaseKind kind, Expression* value, Statement* statement, SourceLocation location);
Statement* createLabeledStatement(const char* labelName, Statement* statement, SourceLocation location);
Statement* createGotoStatement(const char* labelName, SourceLocation location);

// 声明工厂函数
Declaration* createVariableDeclaration(const char* name, TypeSpecifier* type, Expression* initializer, SourceLocation location);
Declaration* createFunctionDeclaration(const char* name, TypeSpecifier* returnType, Vector* parameters, CompoundStatement* body, SourceLocation location);
Declaration* createStructDeclaration(const char* name, Vector* members, SourceLocation location);
Declaration* createUnionDeclaration(const char* name, Vector* members, SourceLocation location);
Declaration* createEnumDeclaration(const char* name, Vector* constants, SourceLocation location);
Declaration* createTypedefDeclaration(const char* name, TypeSpecifier* aliasedType, SourceLocation location);

// 类型说明符工厂函数
TypeSpecifier* createBasicTypeSpecifier(BasicTypeKind kind, SourceLocation location);
TypeSpecifier* createPointerTypeSpecifier(TypeSpecifier* baseType, SourceLocation location);
TypeSpecifier* createArrayTypeSpecifier(TypeSpecifier* elementType, Expression* size, SourceLocation location);
TypeSpecifier* createFunctionTypeSpecifier(TypeSpecifier* returnType, Vector* parameterTypes, bool isVariadic, SourceLocation location);
TypeSpecifier* createStructTypeSpecifier(const char* name, StructDeclaration* declaration, SourceLocation location);
TypeSpecifier* createUnionTypeSpecifier(const char* name, UnionDeclaration* declaration, SourceLocation location);
TypeSpecifier* createEnumTypeSpecifier(const char* name, EnumDeclaration* declaration, SourceLocation location);
TypeSpecifier* createTypedefNameSpecifier(const char* typedefName, SourceLocation location);

// 辅助函数
const char* binaryOperatorToString(BinaryOperator op);
const char* unaryOperatorToString(UnaryOperator op);
const char* assignmentKindToString(AssignmentKind kind);
const char* basicTypeKindToString(BasicTypeKind kind);
const char* storageClassToString(StorageClassSpecifier storage);

#endif // AST_NODES_H
