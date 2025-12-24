#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stdbool.h>
#include <stddef.h>

// 前向声明
typedef struct DiagnosticEngine DiagnosticEngine;
typedef struct HashTable HashTable;
typedef struct Vector Vector;

/**
 * @brief 词法分析器结构体
 *
 * 负责将源代码字符串分解为token序列
 */
typedef struct {
    char* source;           // 源代码
    size_t sourceLength;    // 源代码长度
    size_t position;        // 当前位置
    size_t line;            // 当前行号（从1开始）
    size_t column;          // 当前列号（从1开始）
    size_t lineStartOffset; // 当前行起始偏移量
    char* filename;         // 文件名

    DiagnosticEngine* diagnostics;  // 诊断引擎

    // 关键字表
    HashTable* keywords;

    // 状态标志
    bool inPreprocessor;    // 是否处于预处理状态
    bool inComment;         // 是否处于注释状态
    bool supportUnicode;    // 是否支持Unicode
    bool preserveComments;  // 是否保留注释token

    // 私有数据
    void* privateData;

} Lexer;

// ==================== 构造函数和析构函数 ====================

/**
 * @brief 从源代码字符串创建词法分析器
 * @param source 源代码字符串
 * @param filename 文件名（用于错误报告）
 * @param diagnostics 诊断引擎（可为NULL）
 * @return 新创建的词法分析器，失败返回NULL
 */
Lexer* createLexer(const char* source, const char* filename, DiagnosticEngine* diagnostics);

/**
 * @brief 从文件创建词法分析器
 * @param filename 文件路径
 * @param diagnostics 诊断引擎（可为NULL）
 * @return 新创建的词法分析器，失败返回NULL
 */
Lexer* createLexerFromFile(const char* filename, DiagnosticEngine* diagnostics);

/**
 * @brief 销毁词法分析器
 * @param lexer 要销毁的词法分析器
 */
void destroyLexer(Lexer* lexer);

// ==================== 主要接口 ====================

/**
 * @brief 对整个源代码进行词法分析
 * @param lexer 词法分析器
 * @return token向量（Vector<Token*>），失败返回NULL
 */
Vector* lexerTokenize(Lexer* lexer);

/**
 * @brief 获取下一个token
 * @param lexer 词法分析器
 * @return 下一个token，到达文件末尾时返回EOF token
 */
Token lexerNextToken(Lexer* lexer);

/**
 * @brief 查看下一个token但不移动位置
 * @param lexer 词法分析器
 * @return 下一个token
 */
Token lexerPeekToken(Lexer* lexer);

// ==================== 状态管理 ====================

/**
 * @brief 重置词法分析器到初始状态
 * @param lexer 词法分析器
 */
void lexerReset(Lexer* lexer);

/**
 * @brief 获取当前位置
 * @param lexer 词法分析器
 * @param line 输出行号
 * @param column 输出列号
 */
void lexerGetPosition(const Lexer* lexer, size_t* line, size_t* column);

/**
 * @brief 设置是否保留注释token
 * @param lexer 词法分析器
 * @param preserve true保留，false丢弃
 */
void lexerSetPreserveComments(Lexer* lexer, bool preserve);

/**
 * @brief 设置是否支持Unicode
 * @param lexer 词法分析器
 * @param support true支持，false不支持
 */
void lexerSetSupportUnicode(Lexer* lexer, bool support);

// ==================== 辅助函数 ====================

/**
 * @brief 检查字符串是否为关键字
 * @param lexer 词法分析器
 * @param str 要检查的字符串
 * @return 如果是关键字返回对应的token类型，否则返回TOKEN_IDENTIFIER
 */
TokenType lexerIsKeyword(const Lexer* lexer, const char* str);

/**
 * @brief 获取当前源位置
 * @param lexer 词法分析器
 * @return 当前源位置
 */
SourceLocation lexerGetCurrentLocation(const Lexer* lexer);

/**
 * @brief 跳过空白字符
 * @param lexer 词法分析器
 */
void lexerSkipWhitespace(Lexer* lexer);

/**
 * @brief 跳过注释
 * @param lexer 词法分析器
 * @return 成功跳过返回true，否则返回false
 */
bool lexerSkipComment(Lexer* lexer);

#endif