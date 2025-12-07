#ifndef LEXER_H
#define LEXER_H
#include "token.h"
#include <stdbool.h>
#include <stddef.h>
// 前向声明
typedef struct DiagnosticEngine DiagnosticEngine;
typedef struct HashTable HashTable;
typedef struct Vector Vector;

// 词法分析器结构体
typedef struct {
    char* source;           // 源代码
    size_t sourceLength;    // 源代码长度
    size_t position;        // 当前位置
    size_t line;            // 当前行号
    size_t column;          // 当前列号
    size_t offset;          // 当前行偏移
    char* filename;         // 文件名

    DiagnosticEngine* diagonstics;  //诊断引擎

    // 关键字表
    HashTable* keywords;

    // 状态
    bool inPreprocessor;    // 是否处于预处理状态
    bool inComment;         // 是否处于注释状态
    bool supportUnicode;    // 是否支持Unicode

    // 私有数据
    void* privateData;


} Lexer;

// 构造函数和析构函数
Lexer* createLexer(const char* source, const char* filename, DiagnosticEngine* diagnostics);
Lexer* createLexerFormFile(const char* filename, DiagnosticEngine* diagnostics);
void destroyLexer(Lexer* lexer);

// 主要接口
Vector* lexerTokenize(Lexer* lexer);    //返回Vector<Token>*
Token lexerNextToken(Lexer* lexer);

#endif