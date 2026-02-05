#ifndef AST_DUMPER_H
#define AST_DUMPER_H

#include "ast.h"
#include <stdio.h>
#include <stdbool.h>

// ==================== 前向声明 ====================

/**
 * @brief AST转储器句柄（不透明指针）
 *
 * 隐藏实现细节，只暴露公共API。
 */
typedef struct ASTDumperHandle ASTDumperHandle;

// ==================== 构造函数和析构函数 ====================

/**
 * @brief 创建AST转储器
 * @param output 输出流（NULL表示使用stdout）
 * @return 新创建的转储器，失败返回NULL
 *
 * 创建一个用于输出AST树结构的转储器。
 * 默认配置：
 * - 缩进大小：2个空格
 * - 不显示源位置
 * - 不显示类型信息
 * - 启用颜色输出
 */
ASTDumperHandle* createASTDumper(FILE* output);

/**
 * @brief 销毁AST转储器
 * @param handle 转储器句柄
 */
void destroyASTDumper(ASTDumperHandle* handle);

// ==================== 配置函数 ====================

/**
 * @brief 设置缩进大小
 * @param handle 转储器句柄
 * @param indentSize 每级缩进的空格数
 *
 * 默认值为2。
 */
void astDumperSetIndent(ASTDumperHandle* handle, int indentSize);

/**
 * @brief 设置是否显示源位置
 * @param handle 转储器句柄
 * @param show true显示，false不显示
 *
 * 启用后，每个节点会显示其源文件位置（文件名:行号:列号）。
 */
void astDumperSetShowLocation(ASTDumperHandle* handle, bool show);

/**
 * @brief 设置是否显示类型信息
 * @param handle 转储器句柄
 * @param show true显示，false不显示
 *
 * 启用后，表达式节点会显示其类型信息。
 */
void astDumperSetShowTypes(ASTDumperHandle* handle, bool show);

/**
 * @brief 设置是否使用颜色输出
 * @param handle 转储器句柄
 * @param color true使用颜色，false不使用
 *
 * 启用后，输出会使用ANSI颜色代码高亮显示。
 * 默认启用。
 */
void astDumperSetColorOutput(ASTDumperHandle* handle, bool color);

// ==================== 转储函数 ====================

/**
 * @brief 转储AST树
 * @param handle 转储器句柄
 * @param root AST根节点
 *
 * 使用访问者模式遍历AST树并输出到配置的输出流。
 *
 * 输出格式示例：
 * @code
 * TranslationUnit
 *   FunctionDeclaration: 'main'
 *     ReturnType:
 *       BasicTypeSpecifier: int
 *     Body:
 *       CompoundStatement
 *         Statements (1):
 *           ReturnStatement
 *             LiteralExpr: '0'
 * === Total: 6 nodes ===
 * @endcode
 */
void astDumperDump(ASTDumperHandle* handle, ASTNode* root);

/**
 * @brief 获取上次转储的节点计数
 * @param handle 转储器句柄
 * @return 节点数量
 */
int astDumperGetNodeCount(ASTDumperHandle* handle);

#endif // AST_DUMPER_H
