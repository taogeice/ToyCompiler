#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../common/diagnostics/source_location.h"

//Token类型枚举
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
    //标识符
    TOKEN_IDENTIFIER,

    //字符字面量
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_CHAR_LITERAL,
    TOKEN_STRING_LITERAL,

    //运算符
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_MULTIPLY,     // *
    TOKEN_DIVIDE,       // /
    TOKEN_MODULO,       // %
    TOKEN_ASSIGN,       // =
    TOKEN_PLUS_ASSIGN,  // +=      
    TOKEN_MINUS_ASSIGN, // -=
    TOKEN_MULTIPLY_ASSIGN,  // *=
    TOKEN_DIVIDE_ASSIGN,    // /=
    TOKEN_MODULO_ASSIGN,    // %=
    TOKEN_EQUAL,        // ==
    TOKEN_NOT_EQUAL,    // !=    
    TOKEN_LESS,         // <
    TOKEN_LESS_EQUAL,   // <=
    TOKEN_GREATER,      // >
    TOKEN_GREATER_EQUAL,    // >=
    TOKEN_LOGICAL_AND,  // &&      
    TOKEN_LOGICAL_OR,   // ||
    TOKEN_LOGICAL_NOT,  // !
    TOKEN_BITWISE_AND,  // &
    TOKEN_BITWISE_OR,   // |
    TOKEN_BITWISE_NOT,  // ~
    TOKEN_BITWISE_XOR,  // ^
    TOKEN_LEFT_SHIFT,   // <<
    TOKEN_RIGHT_SHIFT,  // >>
    TOKEN_INCREMENT,    // ++
    TOKEN_DECREMENT,    // --

    //分隔符
    TOKEN_LPAREN,       //(
    TOKEN_RPAREN,       //)
    TOKEN_LBRACE,       //[
    TOKEN_RBRACE,       //]
    TOKEN_SEMICOLON,    //;
    TOKEN_COMMA,        //,
    TOKEN_DOT,          //.
    TOKEN_ARROW,        // ->
    TOKEN_COLON,        //:
    TOKEN_QUESTION,     // ?
    TOKEN_ELLIPSSIS,    // ...
    //特殊标记
    TOKEN_EOF,          //文件结束符
    TOKEN_NEWLINE,      //换行符
    TOKEN_WHITESPACE,   //空格
    TOKEN_COOMENT,      //注释
    TOKEN_UNKNOWN,      //未知

} TokenType;

typedef struct {
    TokenType type;

    char* lexeme;   //token文本
    int line;       //行数
    int column;     //列数
    int length;     //token长度
    SourceLocation location;    //源位置信息
    //字面量值
    union {
        long long intValue;     //整数值
        double floatValue;      //浮点数值
        char charValue;         //字符值
        char* stringValue;      //字符串值
    } value;

    //标志位
    bool hasValue;      //是否有值
    bool isWide;        //是否宽字符

} Token;
//Token构造函数和析构函数
Token* createToken(TokenType type, const char* lexeme, SourceLocation location);
Token* createTokenWithValue(TokenType type, const char* lexeme, SourceLocation location, long long intValue);
Token* createTokenWithFloatValue(TokenType type, const char* lexeme, SourceLocation location, double floatValue);
Token* createTokenWithStringValue(TokenType type, const char* lexeme, SourceLocation location, const char* stringValue, bool isWide);
void destroyToken(Token* token);

//Token辅助函数
const char* tokenTypeToString(TokenType type);
bool tokenIsKeyword(TokenType type);
bool tokenIsOperator(TokenType type);
bool tokenIsLiteral(TokenType type);
bool tokenIsPunctuation(TokenType type);
bool tokenIsAssignmentOperator(TokenType type);

//Token字符串化
char* tokenToString(const Token* token);
void tokenDump(const Token* token, FILE* output);

//源位置函数
SourceLocation createSourceLocation(const char* filename, int line, int column, int offset);
char* sourceLocationToString(const SourceLocation* location);

#endif