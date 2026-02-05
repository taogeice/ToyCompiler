#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "ast.h"
#include "ast_nodes.h"
#include <stdbool.h>

// ==================== 访问者模式接口 ====================

/**
 * @brief AST访问者接口
 *
 * 使用访问者模式实现AST树的双分派（double dispatch）。
 * 这使得可以在不修改AST节点类的情况下添加新的操作。
 *
 * 使用示例：
 * @code
 * typedef struct {
 *     ASTVisitor base;
 *     int indent;
 * } ASTDumper;
 *
 * static void visitLiteralExpr(ASTVisitor* visitor, LiteralExpr* expr) {
 *     ASTDumper* dumper = (ASTDumper*)visitor;
 *     printf("%*sLiteral: %s\n", dumper->indent, "", expr->literalToken->lexeme);
 * }
 *
 * static void visitBinaryOperatorExpr(ASTVisitor* visitor, BinaryOperatorExpr* expr) {
 *     ASTDumper* dumper = (ASTDumper*)visitor;
 *     printf("%*sBinaryOp: %s\n", dumper->indent, "", binaryOperatorToString(expr->op));
 *     dumper->indent += 2;
 *     astNodeAccept((ASTNode*)expr->left, visitor);
 *     astNodeAccept((ASTNode*)expr->right, visitor);
 *     dumper->indent -= 2;
 * }
 * @endcode
 */
typedef struct ASTVisitor {
    // ==================== 翻译单元访问 ====================

    /**
     * @brief 访问翻译单元（AST根节点）
     */
    void (*visitTranslationUnit)(struct ASTVisitor* self, struct TranslationUnit* node);

    // ==================== 表达式访问函数 ====================

    /**
     * @brief 访问字面量表达式
     */
    void (*visitLiteralExpr)(struct ASTVisitor* self, struct LiteralExpr* node);

    /**
     * @brief 访问标识符表达式
     */
    void (*visitIdentifierExpr)(struct ASTVisitor* self, struct IdentifierExpr* node);

    /**
     * @brief 访问二元运算符表达式
     */
    void (*visitBinaryOperatorExpr)(struct ASTVisitor* self, struct BinaryOperatorExpr* node);

    /**
     * @brief 访问一元运算符表达式
     */
    void (*visitUnaryOperatorExpr)(struct ASTVisitor* self, struct UnaryOperatorExpr* node);

    /**
     * @brief 访问赋值表达式
     */
    void (*visitAssignmentExpr)(struct ASTVisitor* self, struct AssignmentExpr* node);

    /**
     * @brief 访问三元条件运算符表达式
     */
    void (*visitTernaryExpr)(struct ASTVisitor* self, struct TernaryExpr* node);

    /**
     * @brief 访问函数调用表达式
     */
    void (*visitFunctionCallExpr)(struct ASTVisitor* self, struct FunctionCallExpr* node);

    /**
     * @brief 访问数组下标表达式
     */
    void (*visitArraySubscriptExpr)(struct ASTVisitor* self, struct ArraySubscriptExpr* node);

    /**
     * @brief 访问成员访问表达式
     */
    void (*visitMemberAccessExpr)(struct ASTVisitor* self, struct MemberAccessExpr* node);

    /**
     * @brief 访问类型转换表达式
     */
    void (*visitCastExpr)(struct ASTVisitor* self, struct CastExpr* node);

    // ==================== 语句访问函数 ====================

    /**
     * @brief 访问表达式语句
     */
    void (*visitExpressionStatement)(struct ASTVisitor* self, struct ExpressionStatement* node);

    /**
     * @brief 访问复合语句（块）
     */
    void (*visitCompoundStatement)(struct ASTVisitor* self, struct CompoundStatement* node);

    /**
     * @brief 访问if语句
     */
    void (*visitIfStatement)(struct ASTVisitor* self, struct IfStatement* node);

    /**
     * @brief 访问while循环语句
     */
    void (*visitWhileStatement)(struct ASTVisitor* self, struct WhileStatement* node);

    /**
     * @brief 访问do-while循环语句
     */
    void (*visitDoWhileStatement)(struct ASTVisitor* self, struct DoWhileStatement* node);

    /**
     * @brief 访问for循环语句
     */
    void (*visitForStatement)(struct ASTVisitor* self, struct ForStatement* node);

    /**
     * @brief 访问return语句
     */
    void (*visitReturnStatement)(struct ASTVisitor* self, struct ReturnStatement* node);

    /**
     * @brief 访问break语句
     */
    void (*visitBreakStatement)(struct ASTVisitor* self, struct BreakStatement* node);

    /**
     * @brief 访问continue语句
     */
    void (*visitContinueStatement)(struct ASTVisitor* self, struct ContinueStatement* node);

    /**
     * @brief 访问switch语句
     */
    void (*visitSwitchStatement)(struct ASTVisitor* self, struct SwitchStatement* node);

    /**
     * @brief 访问case/default标签语句
     */
    void (*visitCaseStatement)(struct ASTVisitor* self, struct CaseStatement* node);

    /**
     * @brief 访问标签语句（用于goto）
     */
    void (*visitLabeledStatement)(struct ASTVisitor* self, struct LabeledStatement* node);

    /**
     * @brief 访问goto语句
     */
    void (*visitGotoStatement)(struct ASTVisitor* self, struct GotoStatement* node);

    // ==================== 声明访问函数 ====================

    /**
     * @brief 访问变量声明
     */
    void (*visitVariableDeclaration)(struct ASTVisitor* self, struct VariableDeclaration* node);

    /**
     * @brief 访问函数声明
     */
    void (*visitFunctionDeclaration)(struct ASTVisitor* self, struct FunctionDeclaration* node);

    /**
     * @brief 访问结构体声明
     */
    void (*visitStructDeclaration)(struct ASTVisitor* self, struct StructDeclaration* node);

    /**
     * @brief 访问联合体声明
     */
    void (*visitUnionDeclaration)(struct ASTVisitor* self, struct UnionDeclaration* node);

    /**
     * @brief 访问枚举声明
     */
    void (*visitEnumDeclaration)(struct ASTVisitor* self, struct EnumDeclaration* node);

    /**
     * @brief 访问typedef声明
     */
    void (*visitTypedefDeclaration)(struct ASTVisitor* self, struct TypedefDeclaration* node);

    // ==================== 类型说明符访问函数 ====================

    /**
     * @brief 访问基础类型说明符
     */
    void (*visitBasicTypeSpecifier)(struct ASTVisitor* self, struct BasicTypeSpecifier* node);

    /**
     * @brief 访问指针类型说明符
     */
    void (*visitPointerTypeSpecifier)(struct ASTVisitor* self, struct PointerTypeSpecifier* node);

    /**
     * @brief 访问数组类型说明符
     */
    void (*visitArrayTypeSpecifier)(struct ASTVisitor* self, struct ArrayTypeSpecifier* node);

    /**
     * @brief 访问函数类型说明符
     */
    void (*visitFunctionTypeSpecifier)(struct ASTVisitor* self, struct FunctionTypeSpecifier* node);

    /**
     * @brief 访问结构体类型说明符
     */
    void (*visitStructTypeSpecifier)(struct ASTVisitor* self, struct StructTypeSpecifier* node);

    /**
     * @brief 访问联合体类型说明符
     */
    void (*visitUnionTypeSpecifier)(struct ASTVisitor* self, struct UnionTypeSpecifier* node);

    /**
     * @brief 访问枚举类型说明符
     */
    void (*visitEnumTypeSpecifier)(struct ASTVisitor* self, struct EnumTypeSpecifier* node);

    /**
     * @brief 访问typedef名称类型说明符
     */
    void (*visitTypedefNameSpecifier)(struct ASTVisitor* self, struct TypedefNameSpecifier* node);

    // ==================== 通用访问函数 ====================

    /**
     * @brief 访问任意AST节点
     * @param self 访问者实例
     * @param node 要访问的节点
     *
     * 这个函数根据节点类型分派到具体的访问函数
     */
    void (*visit)(struct ASTVisitor* self, ASTNode* node);

    /**
     * @brief 访问前钩子函数
     * @param self 访问者实例
     * @param node 即将访问的节点
     * @return true继续访问，false跳过此节点
     *
     * 在访问节点前调用，可用于过滤节点或执行预处理
     */
    bool (*beforeVisit)(struct ASTVisitor* self, ASTNode* node);

    /**
     * @brief 访问后钩子函数
     * @param self 访问者实例
     * @param node 已访问的节点
     *
     * 在访问节点后调用，可用于执行后处理
     */
    void (*afterVisit)(struct ASTVisitor* self, ASTNode* node);

    /**
     * @brief 错误处理钩子函数
     * @param self 访问者实例
     * @param node 出错的节点
     * @param message 错误消息
     *
     * 在访问过程中发生错误时调用
     */
    void (*visitError)(struct ASTVisitor* self, ASTNode* node, const char* message);

    // ==================== 私有数据 ====================

    /**
     * @brief 私有数据指针
     *
     * 访问者实现可以使用这个字段存储特定于访问者的状态
     */
    void* privateData;
} ASTVisitor;

// ==================== AST节点接受函数 ====================

/**
 * @brief AST节点接受访问者
 * @param node AST节点
 * @param visitor 访问者
 *
 * 这是AST节点上的accept方法，实现了访问者模式的第一步分派。
 * 此函数会根据节点类型调用visitor中相应的visit函数。
 *
 * 实现细节：
 * 1. 调用visitor->beforeVisit()（如果存在）
 * 2. 根据node->nodeType调用相应的visit函数
 * 3. 调用visitor->afterVisit()（如果存在）
 */
void astNodeAccept(ASTNode* node, ASTVisitor* visitor);

// ==================== 遍历辅助函数 ====================

/**
 * @brief 深度优先遍历AST树
 * @param root 根节点
 * @param visitor 访问者
 * @param preorder 是否为前序遍历（true）或后序遍历（false）
 *
 * 前序遍历：先访问父节点，再访问子节点
 * 后序遍历：先访问子节点，再访问父节点
 */
void astTraverseDFS(ASTNode* root, ASTVisitor* visitor, bool preorder);

/**
 * @brief 广度优先遍历AST树
 * @param root 根节点
 * @param visitor 访问者
 */
void astTraverseBFS(ASTNode* root, ASTVisitor* visitor);

/**
 * @brief 遍历子节点向量
 * @param children 子节点向量
 * @param visitor 访问者
 *
 * 辅助函数，用于遍历存储在Vector中的子节点
 */
void astTraverseChildren(Vector* children, ASTVisitor* visitor);

// ==================== 访问者工厂函数 ====================

/**
 * @brief 创建空访问者
 * @return 新创建的访问者，失败返回NULL
 *
 * 创建一个所有函数指针都初始化为NULL的访问者。
 * 调用者负责设置所需的访问函数。
 */
ASTVisitor* createASTVisitor(void);

/**
 * @brief 销毁访问者
 * @param visitor 要销毁的访问者
 *
 * 注意：这不会释放visitor->privateData，调用者负责清理私有数据。
 */
void destroyASTVisitor(ASTVisitor* visitor);

/**
 * @brief 复制访问者
 * @param src 源访问者
 * @return 新创建的访问者副本，失败返回NULL
 *
 * 创建访问者的浅拷贝，privateData指针会被复制但不会深拷贝。
 */
ASTVisitor* copyASTVisitor(const ASTVisitor* src);

// ==================== 遍历上下文辅助结构 ====================

/**
 * @brief AST遍历上下文
 *
 * 用于在遍历过程中维护状态的结构
 */
typedef struct {
    int depth;                  // 当前深度
    int maxDepth;               // 最大深度限制（0表示无限制）
    bool visitChildren;         // 是否访问子节点
    bool stopTraversal;         // 是否停止遍历
    void* userData;             // 用户数据
} ASTTraversalContext;

/**
 * @brief 创建遍历上下文
 * @return 新创建的上下文
 */
ASTTraversalContext* createASTTraversalContext(void);

/**
 * @brief 销毁遍历上下文
 * @param context 要销毁的上下文
 *
 * 注意：这不会释放context->userData。
 */
void destroyASTTraversalContext(ASTTraversalContext* context);

/**
 * @brief 重置遍历上下文
 * @param context 要重置的上下文
 */
void resetASTTraversalContext(ASTTraversalContext* context);

// ==================== 内省辅助函数 ====================

/**
 * @brief 获取访问者函数名称
 * @param nodeType AST节点类型
 * @return 访问者函数名称字符串
 */
const char* astVisitorFunctionName(ASTNodeType nodeType);

/**
 * @brief 检查访问者是否实现了特定节点的访问函数
 * @param visitor 访问者
 * @param nodeType AST节点类型
 * @return true如果访问者实现了该节点的访问函数，false否则
 */
bool astVisitorHasHandler(const ASTVisitor* visitor, ASTNodeType nodeType);

/**
 * @brief 获取访问者实现的访问函数数量
 * @param visitor 访问者
 * @return 已实现的访问函数数量
 */
int astVisitorHandlerCount(const ASTVisitor* visitor);

#endif // AST_VISITOR_H
