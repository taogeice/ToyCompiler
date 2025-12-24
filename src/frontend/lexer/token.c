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
    token->literalType = LITERAL_TYPE_DECIMAL;
    token->flags = 0;
    token->userData = NULL;

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
        token->literalType = LITERAL_TYPE_DECIMAL;
    }
    return token;
}

// 创建带浮点值的Token
Token* createTokenWithFloatValue(TokenType type, const char* lexeme, SourceLocation location, double floatValue) {
    Token* token = createToken(type, lexeme, location);
    if (token) {
        token->value.floatValue = floatValue;
        token->hasValue = true;
        token->literalType = LITERAL_TYPE_FLOAT;
    }
    return token;

}

// 创建带字符串值的Token
Token* createTokenWithStringValue(TokenType type, const char* lexeme, SourceLocation location, const char* stringValue, bool isWide) {
    Token* token = createToken(type, lexeme, location);
    if (token) {
        token->value.stringValue = stringValue ? strdup(stringValue) : NULL;
        token->hasValue = true;
        token->isWide = isWide;
        token->literalType = isWide ? LITERAL_TYPE_WSTRING : LITERAL_TYPE_STRING;
    }
    return token;
}

// 创建带字符值的Token
Token* createTokenWithCharValue(TokenType type, const char* lexeme, SourceLocation location, char charValue, bool isWide) {
    Token* token = createToken(type, lexeme, location);
    if (token) {
        token->value.charValue = charValue;
        token->hasValue = true;
        token->isWide = isWide;
        token->literalType = isWide ? LITERAL_TYPE_WCHAR : LITERAL_TYPE_CHAR;
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

// 判断是否为标点符号
bool tokenIsPunctuation(TokenType type) {

    return (type >= TOKEN_ASSIGN && type <= TOKEN_ELLIPSSIS);
}

// 判断是否为赋值运算符
bool tokenIsAssignmentOperator(TokenType type) {
    return (type >= TOKEN_ASSIGN && type <= TOKEN_MODULO_ASSIGN) || type == TOKEN_ASSIGN;
}

// 判断是否为比较运算符
bool tokenIsComparisonOperator(TokenType type) {
    return (type >= TOKEN_EQUAL && type <= TOKEN_GREATER_EQUAL);
}

//判断是否为一元运算符
bool tokenIsUnaryOperator(TokenType type) {

    return (type >= TOKEN_PLUS || type ==TOKEN_MINUS || type == TOKEN_LOGICAL_NOT || type == TOKEN_BITWISE_NOT || type == TOKEN_INCREMENT || type == TOKEN_DECREMENT );
}

// 判断是否为二元运算符
bool tokenIsBinaryOperator(TokenType type) {
    return (type >= TOKEN_PLUS && type <= TOKEN_MODULO) ||
           (type >= TOKEN_EQUAL && type <= TOKEN_LOGICAL_OR) ||
           (type >= TOKEN_BITWISE_AND && type <= TOKEN_RIGHT_SHIFT);
}

// bool转字符串
char* tokenToString(const Token* token) {
    if (!token) {
        return strdup("null");
    }
    const char* typeStr = tokenTypeToString(token->type);
    char* result;

    if (token->lexeme) {
        asprintf(&result, "Token{type=%s, lexeme='%s', line=%d, col=%d}",typeStr, token->lexeme, token->location.line, token->location.column);
    } else {
        asprintf(&result, "Token{type=%s, line=%d, col=%d}", typeStr, token->location.line, token->location.column);
    }
    return result;

}

// Token简短字符串化
char* tokenToShortString(const Token* token) {
    if (!token) {
        return strdup("null");
    }
    const char* typeStr = tokenTypeToString(token->type);
    char* result;
    if (token->lexeme) {
        asprintf(&result, "%s('%s')", typeStr, token->lexeme);
    } else {
        asprintf(&result, "%s", typeStr);
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
// 详细打印token信息
void tokenDumpVerbose(const Token* token, FILE* output) {
    if (!token || !output) {
        return;
    }
    fprintf(output, "=== Token Details ===\n");
    fprintf(output, "Type: %s\n", tokenTypeToString(token->type));
    fprintf(output, "Lexeme: %s\n", token->lexeme ? token->lexeme : "(null)");
    fprintf(output, "Length: %zu\n", token->length);
    fprintf(output, "Location: %s\n", sourceLocationToString(&token->location));
    fprintf(output, "Has Value: %s\n", token->hasValue ? "true" : "false");
    if (token->hasValue) {
        switch (token->type)
        {
        case TOKEN_INTEGER_LITERAL:
            
            fprintf(output, "Integer Value: %lld\n", token->value.intValue);
            break;
        case TOKEN_FLOAT_LITERAL:

            fprintf(output, "Float Value: %f\n", token->value.floatValue);
            break;
        case TOKEN_CHAR_LITERAL:

            fprintf(output, "Char Value: '%c' (wide: %s)\n", token->value.charValue, token->isWide ? "true" : "false");
            break;
        case TOKEN_STRING_LITERAL:

            fprintf(output, "String Value: \%s\" (wide: %s)\n", token->value.stringValue, token->isWide ? "true" : "false");
            break;

        default:
            break;
        }

        fprintf(output, "Is Wide: %s\n", token->isWide ? "true" : "false");
        fprintf(output, "Literal Type: %d\n", token->literalType);
        fprintf(output, "Flags: %d\n", token->flags);
        fprintf(output, "=====================\n");

    }
}
// 创建源位置信息
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
        asprintf(&result, "%s:%d:%d", location->filename,location->line,location->column);
    } else {
        asprintf(&result, "line %d, column %d", location->line, location->column);
    }
    return result;
}

// 销毁源位置信息
void destroySourceLocation(SourceLocation* location) {
    if (location && location->filename) {
        free(location->filename);
        location->filename = NULL;
    }
}

// token工厂函数实现
Token* createEOFToken(SourceLocation location) {
    return createToken(TOKEN_EOF, NULL, location);
}

Token* createIdentifierToken(const char* identifier, SourceLocation location) {
    return createToken(TOKEN_IDENTIFIER, identifier, location);
}

Token* createIntegerToken(const char* lexeme, SourceLocation location, int base) {
    long long value = strtoll(lexeme, NULL, base);
    Token* token = createTokenWithValue(TOKEN_INTEGER_LITERAL, lexeme, location, value);
    if (token) {
        switch (base)
        {
        case 16: token->literalType = LITERAL_TYPE_HEXADECIMAL;
            break;
        case 8: token->literalType = LITERAL_TYPE_OCTAL;
            break;
        case 2: token->literalType = LITERAL_TYPE_BINARY;
            break;
        default:
            token->literalType = LITERAL_TYPE_DECIMAL;
            break;
        }
    }
    return token;
}

// 创建浮点数Token
Token* createFloatToken(const char* lexeme, SourceLocation location) {
    double value = strtod(lexeme, NULL);
    Token* token = createTokenWithFloatValue(TOKEN_FLOAT_LITERAL, lexeme, location, value);
    if (token) {
        token->literalType = LITERAL_TYPE_DOUBLE;
    }
    return token;
}

// 创建字符Token
Token* createCharToken(const char* lexeme, SourceLocation location) {
    // 从词素中提取字符值（去掉引号）
    char value = '\0';
    if (lexeme && strlen(lexeme) >= 3) {
        // 简单处理，实际需要处理转义序列
        if (lexeme[0] == '\'' && lexeme[2] == '\'') {
            value = lexeme[1];
        }
    }
    return createTokenWithCharValue(TOKEN_CHAR_LITERAL, lexeme, location, value, false);
}

// 创建字符串Token
Token* createStringToken(const char* lexeme, SourceLocation location, bool isWide) {
    // 从词素中提取字符串值（去掉引号）
    const char* value = "";
    if (lexeme && strlen(lexeme) >= 2) {
        size_t len = strlen(lexeme);
        if ((lexeme[0] == '"' || (isWide && lexeme[0] == 'L')) && lexeme[len - 1] == '"') {
            // 简单处理：去掉引号，实际需要处理转义序列
            if (isWide && len >= 3) {
                value = lexeme + 2;  // 跳过L"
            } else {
                value = lexeme + 1;  // 跳过"
            }
        }
    }
    return createTokenWithStringValue(TOKEN_STRING_LITERAL, lexeme, location, value, isWide);
}

// 创建运算符Token
Token* createOperatorToken(TokenType type, SourceLocation location) {
    const char* lexeme = tokenTypeToString(type);
    return createToken(type, lexeme, location);
}

// 创建标点符号Token
Token* createPunctuationToken(TokenType type, SourceLocation location) {
    const char* lexeme = tokenTypeToString(type);
    return createToken(type, lexeme, location);
}

// ==================== Token验证函数 ====================

// 验证token有效性
bool tokenIsValid(const Token* token) {
    if (!token) {
        return false;
    }
    // 检查token类型是否有效
    if (token->type < TOKEN_INT || token->type > TOKEN_UNKNOWN) {
        return false;
    }
    return true;
}

// 验证token位置有效性
bool tokenHasValidLocation(const Token* token) {
    if (!token) {
        return false;
    }
    // 行号和列号应该大于0
    return (token->location.line > 0 && token->location.column > 0);
}

// 验证token词素有效性
bool tokenHasValidLexeme(const Token* token) {
    if (!token) {
        return false;
    }
    // EOF token可以没有词素
    if (token->type == TOKEN_EOF) {
        return true;
    }
    // 其他token应该有词素
    return (token->lexeme != NULL && token->length > 0);
}

// ==================== Token比较函数 ====================

// 比较两个token是否相等
bool tokenEquals(const Token* token1, const Token* token2) {
    if (!token1 || !token2) {
        return (token1 == token2);
    }
    // 比较类型
    if (token1->type != token2->type) {
        return false;
    }
    // 比较词素
    if (token1->lexeme && token2->lexeme) {
        if (strcmp(token1->lexeme, token2->lexeme) != 0) {
            return false;
        }
    } else if (token1->lexeme != token2->lexeme) {
        return false;
    }
    return true;
}

// 比较token类型是否相等
bool tokenTypeEquals(const Token* token1, TokenType type) {
    if (!token1) {
        return false;
    }
    return (token1->type == type);
}

// 比较token词素是否相等
bool tokenLexemeEquals(const Token* token, const char* lexeme) {
    if (!token || !lexeme) {
        return false;
    }
    if (!token->lexeme) {
        return false;
    }
    return (strcmp(token->lexeme, lexeme) == 0);
}

// ==================== Token标志位操作函数 ====================

// 设置标志位
void tokenSetFlag(Token* token, unsigned int flag) {
    if (token) {
        token->flags |= flag;
    }
}

// 清除标志位
void tokenClearFlag(Token* token, unsigned int flag) {
    if (token) {
        token->flags &= ~flag;
    }
}

// 检查标志位
bool tokenHasFlag(const Token* token, unsigned int flag) {
    if (!token) {
        return false;
    }
    return ((token->flags & flag) != 0);
}