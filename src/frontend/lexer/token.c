#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Token类型到字符串的映射表
static const char* tokenTypeStrings[] = {
    //关键字
    "int", "float", "char", "double", "void", "if", "else", "while", "for", "do",
    "return", "break", "continue", "switch", "case", "default", "struct", "union",
    "enum", "typedef", "static", "extern", "const", "unsigned", "signed", "sizeof",
    "auto", "register", "volatile", "goto", "alignas", "alignof", "atomic", "generic",
    "static_assert", "thread_local", "noreturn",
    //标识符和字面量
    "identifier", "integer_literal", "float_literal", "char_literal", "string_literal",
    //运算符
    "+", "-", "*", "/", "%", "=", "+=", "-=", "*=", "/=", "%=", "==", "!=", "<", "<=",
    ">", ">=", "&&", "||", "!", "&", "|", "~", "^", "<<", ">>", "++", "--",
    //分隔符
    "{", "}", "(", ")", "[", "]", ".", ",", ";", ":", "?", "...", "->", "->*",
    // 特殊标记
    "eof", "newline", "whitespace", "comment", "preprocessor", "unknown"
};

// 创建token
Token* createToken(TokenType type, const char* lexeme, SourceLocation location) {
    Token* token = (Token*)malloc(sizeof(Token));
    if (!token) {

        return NULL;
    }
    token->type = type;
    token->location = location;
    token->hasValue = false;
    token->isWide = false;
    if (lexeme) {
        token->lexeme = strdup(lexeme);
        token->length = strlen(lexeme);

    } else {
        token->lexeme = NULL;
        token->length = 0;

    }

    return token;

}

// 创建带整数值的Token
Token* createTokenWithValue(TokenType type, const char* lexeme, SourceLocation location, long long intValue) {
    Token* token = createToken(type, lexeme, location);
    if (token) {
        token->value.intValue = intValue;
        token->hasValue = true;

    }
    return token;
}

// 创建带浮点值的Token
Token* createTokenWithFloatValue(TokenType type, const char* lexeme, SourceLocation location, double floatValue) {
    Token* token = createToken(type, lexeme, location);
    if (token) {
        token->value.floatValue = floatValue;
        token->hasValue = true;

    }
    return token;

}

// 创建带字符串值的Token
Token* createTokenWithStringValue(TokenType type, const char* lexeme, SourceLocation location, const char* stringValue, bool isWide) {
    Token* token = createToken(type, lexeme, location);
    if (token) {
        token->value.stringValue = stringValue;
        token->hasValue = true;
        token->isWide = isWide;


    }

    return token;
}
// 销毁token
void destroyToken(Token* token){
    if (token) {
        if (token->lexeme) {
            free(token->lexeme);
        }
        if (token->hasValue && (token->type == TOKEN_STRING_LITERAL || token->type == TOKEN_CHAR_LITERAL)) {
            if (token->value.stringValue) {
                free(token->value.stringValue);
            }

        }
        free(token);
    }

}
// Token类型转字符串
const char* tokenTypeToString(TokenType type) {
    if (type >= 0 && type < sizeof(tokenTypeStrings) / sizeof(tokenTypeStrings[0])) {
        return tokenTypeStrings[type];

    }
    return "unknown";
}

// 判断是否为关键字
bool tokenIsKeyword(TokenType type) {
    return (type >= TOKEN_INT && type <= TOKEN_NORETURN);
}

// 运算符判断
bool tokenIsOperator(TokenType type) {
    return (type >= TOKEN_PLUS && type <= TOKEN_DECREMENT);
}
// 判断是否为字面量
bool tokenIsLiteral(TokenType type) {
    return (type >= TOKEN_INTEGER_LITERAL && type <= TOKEN_STRING_LITERAL);
}
// 判断是否为赋值运算符
bool tokenIsAssignmentOperator(TokenType type) {
    return (type >= TOKEN_ASSIGN && type <= TOKEN_MODULO_ASSIGN) || type == TOKEN_ASSIGN;
}

// bool转字符串
char* tokenToString(const Token* token) {
    if (!token) {
        return strdup("null");
    }
    const char* typeStr = tokenTypeString(token->type);
    char* result;

    if (token->lexeme) {
        asprintf(&result, "Token{type=%s, lexeme='%s', line=%d, col=%d}",
        typeStr, token->lexeme, token->location.line, token->location.column);
    } else {
        asprintf(&result, "Token{type=%s, line=%d, col=%d}", typeStr, token->location.line, token->column);

    }
    return result;

}

// 打印token信息
void tokenDump(const Token* token, FILE* output) {
    if (!token || !output){
        return;
    }
    char* tokenStr = tokenToString(token);
    fprintf(output, "%s\n", tokenStr);
    free(tokenStr);

}
//创建源位置信息
SourceLocation createSourceLocation(const char* filename, int line,int column, int offset) {
    SourceLocation loc;
    loc.filename = filename ? strdup(filename) : NULL;
    loc.line = line;
    loc.column = column;
    loc.offset = offset;
    return loc;
}

// 源位置转字符串
char* sourceLocationToString(const SourceLocation* location) {
    if (!location) {
        return strdup("unknown");
    }
    char* result;
    if(location->filename){
        asprintf(&result, "%ss:%d:%d", location->filename,location->line,location->column);
    } else {
        asprintf(&result, "line %d, column %d", location->line, location->column);
    }
    return result;
}

