#ifndef AST_UTILS_H
#define AST_UTILS_H

#include "ast.h"
#include "ast_nodes.h"
#include "../../common/diagnostics/diagnostic_engine.h"
#include <stdbool.h>
#include <stddef.h>

// ==================== 节点信息函数 ====================

/**
 * @brief 计算节点的后代节点数量（包括自身）
 * @param node AST节点
 * @return 后代节点总数
 *
 * 递归计算以该节点为根的子树中的所有节点数量。
 */
size_t astNodeCountDescendants(const ASTNode* node);

/**
 * @brief 计算AST树的深度
 * @param root 根节点
 * @return 树的深度
 *
 * 从根节点到最远叶子节点的最长路径长度。
 */
size_t astTreeGetDepth(const ASTNode* root);

/**
 * @brief 检查节点是否有父节点
 * @param node AST节点
 * @return true如果有父节点，false否则
 */
bool astNodeHasParent(const ASTNode* node);

/**
 * @brief 检查节点是否为根节点
 * @param node AST节点
 * @return true如果是根节点，false否则
 */
bool astNodeIsRoot(const ASTNode* node);

/**
 * @brief 获取节点的直接子节点数量
 * @param node AST节点
 * @return 直接子节点数量
 */
size_t astNodeGetChildCount(const ASTNode* node);

// ==================== 类型检查函数 ====================

/**
 * @brief 检查节点是否为表达式
 * @param node AST节点
 * @return true如果是表达式，false否则
 *
 * 表达式节点包括：
 * - LiteralExpr
 * - IdentifierExpr
 * - BinaryOperatorExpr
 * - UnaryOperatorExpr
 * - AssignmentExpr
 * - TernaryExpr
 * - FunctionCallExpr
 * - ArraySubscriptExpr
 * - MemberAccessExpr
 * - CastExpr
 */
bool astNodeIsExpression(const ASTNode* node);

/**
 * @brief 检查节点是否为语句
 * @param node AST节点
 * @return true如果是语句，false否则
 *
 * 语句节点包括：
 * - ExpressionStatement
 * - CompoundStatement
 * - IfStatement
 * - WhileStatement
 * - DoWhileStatement
 * - ForStatement
 * - ReturnStatement
 * - BreakStatement
 * - ContinueStatement
 * - SwitchStatement
 * - CaseStatement
 * - LabeledStatement
 * - GotoStatement
 */
bool astNodeIsStatement(const ASTNode* node);

/**
 * @brief 检查节点是否为声明
 * @param node AST节点
 * @return true如果是声明，false否则
 *
 * 声明节点包括：
 * - VariableDeclaration
 * - FunctionDeclaration
 * - StructDeclaration
 * - UnionDeclaration
 * - EnumDeclaration
 * - TypedefDeclaration
 */
bool astNodeIsDeclaration(const ASTNode* node);

/**
 * @brief 检查节点是否为类型说明符
 * @param node AST节点
 * @return true如果是类型说明符，false否则
 *
 * 类型说明符节点包括：
 * - BasicTypeSpecifier
 * - PointerTypeSpecifier
 * - ArrayTypeSpecifier
 * - FunctionTypeSpecifier
 * - StructTypeSpecifier
 * - UnionTypeSpecifier
 * - EnumTypeSpecifier
 * - TypedefNameSpecifier
 */
bool astNodeIsTypeSpecifier(const ASTNode* node);

// ==================== 节点遍历函数 ====================

/**
 * @brief 遍历AST树
 * @param root 根节点
 * @param visitor 访问者
 *
 * 使用访问者模式遍历以root为根的AST树。
 * 等价于调用 astNodeAccept(root, visitor)。
 */
void astNodeTraverse(ASTNode* root, ASTVisitor* visitor);

// ==================== 节点查找函数 ====================

/**
 * @brief 查找指定类型的直接子节点
 * @param node 父节点
 * @param type 要查找的节点类型
 * @return 匹配的节点向量（ASTNode*），调用者负责销毁
 *
 * 只查找直接子节点，不递归查找。
 * 如果没有找到，返回空向量。
 * 失败返回NULL。
 */
Vector* astNodeFindChildrenByType(const ASTNode* node, ASTNodeType type);

/**
 * @brief 查找指定类型的第一个祖先节点
 * @param node 起始节点
 * @param type 要查找的节点类型
 * @return 找到的祖先节点，未找到返回NULL
 *
 * 沿着父指针链向上查找，返回第一个匹配的祖先节点。
 */
ASTNode* astNodeFindParentByType(const ASTNode* node, ASTNodeType type);

/**
 * @brief 查找指定类型的所有后代节点
 * @param root 根节点
 * @param type 要查找的节点类型
 * @return 匹配的节点向量（ASTNode*），调用者负责销毁
 *
 * 递归查找所有后代节点。
 * 如果没有找到，返回空向量。
 * 失败返回NULL。
 */
Vector* astNodeFindAllByType(ASTNode* root, ASTNodeType type);

// ==================== 节点验证函数 ====================

/**
 * @brief 验证单个AST节点
 * @param node 要验证的节点
 * @param diagnostics 诊断引擎（可以为NULL）
 * @return true如果节点有效，false否则
 *
 * 验证内容包括：
 * - 必需字段是否存在
 * - 指针有效性
 * - 基本语义约束
 *
 * 如果提供diagnostics，会报告所有发现的错误。
 */
bool astNodeValidate(const ASTNode* node, DiagnosticEngine* diagnostics);

/**
 * @brief 验证整个AST树
 * @param root 根节点
 * @param diagnostics 诊断引擎（可以为NULL）
 * @return true如果树有效，false否则
 *
 * 递归验证树中的所有节点。
 * 如果提供diagnostics，会报告所有发现的错误。
 */
bool astTreeValidate(const ASTNode* root, DiagnosticEngine* diagnostics);

// ==================== 节点克隆函数 ====================

/**
 * @brief 克隆AST节点（深度复制）
 * @param node 要克隆的节点
 * @return 新克隆的节点，失败返回NULL
 *
 * 创建节点的深拷贝，包括所有子节点。
 * 调用者负责销毁返回的节点。
 */
ASTNode* astNodeClone(const ASTNode* node);

// ==================== 节点比较函数 ====================

/**
 * @brief 比较两个AST节点是否相等
 * @param a 第一个节点
 * @param b 第二个节点
 * @return true如果节点相等，false否则
 *
 * 深度比较两个节点及其所有子节点。
 */
bool astNodeEquals(const ASTNode* a, const ASTNode* b);

#endif // AST_UTILS_H
