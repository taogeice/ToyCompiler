#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "../../common/diagnostics/source_location.h"

// Token类型枚举
typedef enum {
    // 关键字
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_CHAR,
    TOKEN_DOUBLE,
    TOKEN_VOID,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_DO,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_STRUCT,
    TOKEN_UNION,
    TOKEN_ENUM,
    TOKEN_TYPEDEF,
    TOKEN_STATIC,
    TOKEN_EXTERN,
    TOKEN_CONST,
    TOKEN_UNSIGNED,
    TOKEN_SIGNED,
    TOKEN_SIZEOF,
    TOKEN_AUTO,
    TOKEN_REGISTER,
    TOKEN_VOLATILE,
    TOKEN_GOTO,
    
    // C11/C17 新增关键字
    TOKEN_ALIGNAS,
    TOKEN_ALIGNOF,
    TOKEN_ATOMIC,
    TOKEN_GENERIC,
    TOKEN_STATIC_ASSERT,
    TOKEN_THREAD_LOCAL,
    TOKEN_NORETURN,
    // 标识符
    TOKEN_IDENTIFIER,

    // 字符字面量
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_CHAR_LITERAL,
    TOKEN_STRING_LITERAL,

    // 一元运算符
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_MULTIPLY,     // *
    TOKEN_DIVIDE,       // /
    TOKEN_MODULO,       // %
    TOKEN_ASSIGN,       // =

    // 赋值运算符
    TOKEN_PLUS_ASSIGN,  // +=      
    TOKEN_MINUS_ASSIGN, // -=
    TOKEN_MULTIPLY_ASSIGN,  // *=
    TOKEN_DIVIDE_ASSIGN,    // /=
    TOKEN_MODULO_ASSIGN,    // %=

    // 比较符
    TOKEN_EQUAL,        // ==
    TOKEN_NOT_EQUAL,    // !=    
    TOKEN_LESS,         // <
    TOKEN_LESS_EQUAL,   // <=
    TOKEN_GREATER,      // >
    TOKEN_GREATER_EQUAL,    // >=

    // 逻辑运算符
    TOKEN_LOGICAL_AND,  // &&      
    TOKEN_LOGICAL_OR,   // ||
    TOKEN_LOGICAL_NOT,  // !
    TOKEN_BITWISE_AND,  // &
    TOKEN_BITWISE_OR,   // |
    TOKEN_BITWISE_NOT,  // ~
    TOKEN_BITWISE_XOR,  // ^
    TOKEN_LEFT_SHIFT,   // <<
    TOKEN_RIGHT_SHIFT,  // >>

    // 自增自减
    TOKEN_INCREMENT,    // ++
    TOKEN_DECREMENT,    // --

    // 分隔符
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACKET,     // [
    TOKEN_RBRACKET,     // ]
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_SEMICOLON,    // ;
    TOKEN_COMMA,        // ,
    TOKEN_DOT,          // .
    TOKEN_ARROW,        // ->
    TOKEN_COLON,        // :
    TOKEN_QUESTION,     // ?
    TOKEN_ELLIPSSIS,    // ...
    
    // 特殊标记
    TOKEN_EOF,          // 文件结束符
    TOKEN_NEWLINE,      // 换行符
    TOKEN_WHITESPACE,   // 空格
    TOKEN_COMMENT,      // 注释
    TOKEN_UNKNOWN,      // 未知

    // 预处理指令
    TOKEN_HASH,         // #
    TOKEN_HASH_HASH,    // ##
    TOKEN_PREPROCESSOR_DEFINE,     // #define
    TOKEN_PREPROCESSOR_UNDEF,      // #undef
    TOKEN_PREPROCESSOR_INCLUDE,    // #include
    TOKEN_PREPROCESSOR_IF,         // #if
    TOKEN_PREPROCESSOR_IFDEF,      // #ifdef
    TOKEN_PREPROCESSOR_IFNDEF,     // #ifndef
    TOKEN_PREPROCESSOR_ELIF,       // #elif
    TOKEN_PREPROCESSOR_ELSE,       // #else
    TOKEN_PREPROCESSOR_ENDIF,      // #endif
    TOKEN_PREPROCESSOR_LINE,       // #line
    TOKEN_PREPROCESSOR_ERROR,      // #error
    TOKEN_PREPROCESSOR_PRAGMA,     // #pragma
    TOKEN_PREPROCESSOR_WARNING,    // #warning

} TokenType;


// 字面量类型子枚举
typedef enum {
    LITERAL_TYPE_DECIMAL,
    LITERAL_TYPE_HEXADECIMAL,
    LITERAL_TYPE_OCTAL,
    LITERAL_TYPE_BINARY,
    LITERAL_TYPE_FLOAT,
    LITERAL_TYPE_DOUBLE,
    LITERAL_TYPE_CHAR,
    LITERAL_TYPE_WCHAR,
    LITERAL_TYPE_STRING,
    LITERAL_TYPE_WSTRING,

} LiteralType;


typedef struct {
    TokenType type;

    char* lexeme;   // token文本
    size_t length;     // token长度
    SourceLocation location;    // 源位置信息
    // 字面量值
    union {
        long long intValue;     // 整数值
        double floatValue;      // 浮点数值
        char charValue;         // 字符值
        char* stringValue;      // 字符串值
    } value;

    //标志位
    bool hasValue;      // 是否有值
    bool isWide;        // 是否宽字符
    LiteralType literalType;    // 字面量类型

    // 拓展信息
    unsigned int flags; // 拓展标志位   
    void* userData;     // 用户数据指针

} Token;


// Token构造函数和析构函数
Token* createToken(TokenType type, const char* lexeme, SourceLocation location);
Token* createTokenWithValue(TokenType type, const char* lexeme, SourceLocation location, long long intValue);
Token* createTokenWithFloatValue(TokenType type, const char* lexeme, SourceLocation location, double floatValue);
Token* createTokenWithStringValue(TokenType type, const char* lexeme, SourceLocation location, const char* stringValue, bool isWide);
Token* createTokenWithCharValue(TokenType type, const char* lexeme, SourceLocation location, char charValue, bool isWide);
void destroyToken(Token* token);

// Token辅助函数
const char* tokenTypeToString(TokenType type);
bool tokenIsKeyword(TokenType type);
bool tokenIsOperator(TokenType type);
bool tokenIsLiteral(TokenType type);
bool tokenIsPunctuation(TokenType type);
bool tokenIsAssignmentOperator(TokenType type);
bool tokenIsComparisonOperator(TokenType type);
bool tokenIsUnaryOperator(TokenType type);
bool tokenIsBinaryOperator(TokenType type);


// Token字符串化
char* tokenToString(const Token* token);
char* tokenToShortString(const Token* token);
void tokenDump(const Token* token, FILE* output);
void tokenDumpVerbose(const Token* token, FILE* output);

// 源位置函数
SourceLocation createSourceLocation(const char* filename, int line, int column, int offset);
char* sourceLocationToString(const SourceLocation* location);
void destroySourceLocation(SourceLocation* location);

// Token工厂函数
Token* createEOFToken(SourceLocation location);
Token* createIdentifierToken(const char* identifier, SourceLocation location);
Token* createIntegerToken(const char* lexeme, SourceLocation location, int base);
Token* createFloatToken(const char* lexeme, SourceLocation location);
Token* createCharToken(const char* lexeme, SourceLocation location);
Token* createStringToken(const char* lexeme, SourceLocation location, bool isWide);
Token* createOperatorToken(TokenType type, SourceLocation location);
Token* createPunctuationToken(TokenType type, SourceLocation location);


// Token验证函数
bool tokenIsValid(const Token* token);
bool tokenHasValidLocation(const Token* token);
bool tokenHasValidLexeme(const Token* token);

// Token比较函数
bool tokenEquals(const Token* token1, const Token* token2);
bool tokenTypeEquals(const Token* token1, TokenType type);
bool tokenLexemeEquals(const Token* token, const char* lexeme);


// Token集合操作
void tokenSetFlag(Token* token, unsigned int flag);
void tokenClearFlag(Token* token, unsigned int flag);
bool tokenHasFlag(const Token* token, unsigned int flag);


// 预定义的标志位
#define TOKEN_FLAG_ESCAPE_SEQUENCE (1 << 0) //包含转义序列
#define TOKEN_FLAG_RAW_STRING (1 << 1)  //原始字符串
#define TOKEN_FLAG_UNICODE (1 << 2) // Unicode字符串
#define TOKEN_FLAG_PREPROCESSOR (1 << 3)    // 预处理



#endif