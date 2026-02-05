#include "lexer.h"
#include "../../common/containers/vector.h"
#include "../../common/diagnostics/diagnostic_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

// ==================== 内部辅助类型 ====================

/**
 * @brief 关键字表项
 */
typedef struct {
    const char* keyword;
    TokenType tokenType;
} KeywordEntry;

/**
 * @brief 预处理指令表项
 */
typedef struct {
    const char* directive;
    TokenType tokenType;
} PreprocessorDirectiveEntry;

/**
 * @brief 关键字映射表
 * 注意：必须与token.h中的TokenType枚举顺序一致
 */
static const KeywordEntry keywordTable[] = {
    // C基础关键字
    {"int", TOKEN_INT},
    {"float", TOKEN_FLOAT},
    {"char", TOKEN_CHAR},
    {"double", TOKEN_DOUBLE},
    {"void", TOKEN_VOID},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"do", TOKEN_DO},
    {"return", TOKEN_RETURN},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"switch", TOKEN_SWITCH},
    {"case", TOKEN_CASE},
    {"default", TOKEN_DEFAULT},
    {"struct", TOKEN_STRUCT},
    {"union", TOKEN_UNION},
    {"enum", TOKEN_ENUM},
    {"typedef", TOKEN_TYPEDEF},
    {"static", TOKEN_STATIC},
    {"extern", TOKEN_EXTERN},
    {"const", TOKEN_CONST},
    {"unsigned", TOKEN_UNSIGNED},
    {"signed", TOKEN_SIGNED},
    {"sizeof", TOKEN_SIZEOF},
    {"auto", TOKEN_AUTO},
    {"register", TOKEN_REGISTER},
    {"volatile", TOKEN_VOLATILE},
    {"goto", TOKEN_GOTO},
    // C11/C17 新增关键字
    {"alignas", TOKEN_ALIGNAS},
    {"_Alignas", TOKEN_ALIGNAS},
    {"alignof", TOKEN_ALIGNOF},
    {"_Alignof", TOKEN_ALIGNOF},
    {"atomic", TOKEN_ATOMIC},
    {"_Atomic", TOKEN_ATOMIC},
    {"generic", TOKEN_GENERIC},
    {"_Generic", TOKEN_GENERIC},
    {"static_assert", TOKEN_STATIC_ASSERT},
    {"_Static_assert", TOKEN_STATIC_ASSERT},
    {"thread_local", TOKEN_THREAD_LOCAL},
    {"_Thread_local", TOKEN_THREAD_LOCAL},
    {"noreturn", TOKEN_NORETURN},
    {"_Noreturn", TOKEN_NORETURN},
    {NULL, TOKEN_IDENTIFIER}  // 哨兵值
};

#define KEYWORD_COUNT (sizeof(keywordTable) / sizeof(KeywordEntry) - 1)

/**
 * @brief 预处理指令映射表
 */
static const PreprocessorDirectiveEntry preprocessorDirectiveTable[] = {
    {"define", TOKEN_PREPROCESSOR_DEFINE},
    {"undef", TOKEN_PREPROCESSOR_UNDEF},
    {"include", TOKEN_PREPROCESSOR_INCLUDE},
    {"if", TOKEN_PREPROCESSOR_IF},
    {"ifdef", TOKEN_PREPROCESSOR_IFDEF},
    {"ifndef", TOKEN_PREPROCESSOR_IFNDEF},
    {"elif", TOKEN_PREPROCESSOR_ELIF},
    {"else", TOKEN_PREPROCESSOR_ELSE},
    {"endif", TOKEN_PREPROCESSOR_ENDIF},
    {"line", TOKEN_PREPROCESSOR_LINE},
    {"error", TOKEN_PREPROCESSOR_ERROR},
    {"pragma", TOKEN_PREPROCESSOR_PRAGMA},
    {"warning", TOKEN_PREPROCESSOR_WARNING},
    {NULL, TOKEN_HASH}  // 哨兵值
};

#define PREPROCESSOR_DIRECTIVE_COUNT (sizeof(preprocessorDirectiveTable) / sizeof(PreprocessorDirectiveEntry) - 1)

// ==================== 内部辅助函数 ====================

/**
 * @brief 报告词法错误
 */
static void lexerReportError(Lexer* lexer, LexerErrorType errorType,
                             SourceLocation location, const char* format, ...) {
    if (!lexer || !lexer->diagnostics) {
        return;
    }

    va_list args;
    va_start(args, format);

    // 计算所需缓冲区大小
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    char* message = NULL;
    if (len >= 0) {
        message = (char*)malloc(len + 1);
        if (message) {
            vsnprintf(message, len + 1, format, args);
        }
    }
    va_end(args);

    // 根据错误类型确定诊断级别
    DiagnosticLevel level = DIAGNOSTIC_LEVEL_ERROR;
    switch (errorType) {
        case LEX_ERROR_INVALID_CHARACTER:
        case LEX_ERROR_INVALID_ESCAPE_SEQUENCE:
        case LEX_ERROR_INVALID_NUMBER_FORMAT:
        case LEX_ERROR_INVALID_UNICODE:
            level = DIAGNOSTIC_LEVEL_ERROR;
            break;
        case LEX_ERROR_UNTERMINATED_COMMENT:
        case LEX_ERROR_UNTERMINATED_CHAR:
        case LEX_ERROR_UNTERMINATED_STRING:
        case LEX_ERROR_EOF_IN_PREPROCESSOR:
        case LEX_ERROR_MISMATCHED_BRACKET:
            level = DIAGNOSTIC_LEVEL_FATAL;
            break;
    }

    diagnosticEngineReport(lexer->diagnostics, level, location,
                          "lexer: %s", message ? message : "unknown error");

    if (message) {
        free(message);
    }
}

/**
 * @brief 报告词法警告
 */
static void lexerReportWarning(Lexer* lexer, SourceLocation location,
                              const char* format, ...) {
    if (!lexer || !lexer->diagnostics) {
        return;
    }

    va_list args;
    va_start(args, format);

    // 计算所需缓冲区大小
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    char* message = NULL;
    if (len >= 0) {
        message = (char*)malloc(len + 1);
        if (message) {
            vsnprintf(message, len + 1, format, args);
        }
    }
    va_end(args);

    diagnosticEngineReport(lexer->diagnostics, DIAGNOSTIC_LEVEL_WARNING, location,
                          "lexer: %s", message ? message : "unknown warning");

    if (message) {
        free(message);
    }
}

/**
 * @brief 获取当前字符
 */
static inline char lexerCurrentChar(const Lexer* lexer) {
    if (lexer->position >= lexer->sourceLength) {
        return '\0';
    }
    return lexer->source[lexer->position];
}

/**
 * @brief 获取下一个字符但不移动位置
 */
static inline char lexerPeekChar(const Lexer* lexer, size_t offset) {
    size_t pos = lexer->position + offset;
    if (pos >= lexer->sourceLength) {
        return '\0';
    }
    return lexer->source[pos];
}

/**
 * @brief 获取下一个字符（peek1的别名）
 */
static inline char lexerPeekNext(const Lexer* lexer) {
    return lexerPeekChar(lexer, 1);
}

/**
 * @brief 前进一个字符
 */
static inline char lexerAdvance(Lexer* lexer) {
    if (lexer->position >= lexer->sourceLength) {
        return '\0';
    }
    char ch = lexer->source[lexer->position++];
    lexer->column++;

    // 处理换行符
    if (ch == '\n') {
        lexer->line++;
        lexer->column = 1;
        lexer->lineStartOffset = lexer->position;
    }

    return ch;
}

/**
 * @brief 检查是否到达文件末尾
 */
static inline bool lexerIsAtEnd(const Lexer* lexer) {
    return lexer->position >= lexer->sourceLength;
}

/**
 * @brief 匹配当前字符，如果匹配则前进
 */
static inline bool lexerMatchChar(Lexer* lexer, char expected) {
    if (lexerIsAtEnd(lexer)) {
        return false;
    }
    if (lexerCurrentChar(lexer) != expected) {
        return false;
    }
    lexerAdvance(lexer);
    return true;
}

/**
 * @brief 跳过当前字符
 */
static inline void lexerSkipChar(Lexer* lexer) {
    lexerAdvance(lexer);
}

/**
 * @brief 创建当前源位置的Token位置信息
 */
static inline SourceLocation lexerCreateCurrentLocation(const Lexer* lexer) {
    return createSourceLocation(
        lexer->filename,
        (int)lexer->line,
        (int)lexer->column,
        (int)lexer->position
    );
}

// ==================== 构造函数和析构函数 ====================

/**
 * @brief 从源代码字符串创建词法分析器
 */
Lexer* createLexer(const char* source, const char* filename, DiagnosticEngine* diagnostics) {
    if (!source) {
        return NULL;
    }

    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) {
        return NULL;
    }

    // 复制源代码
    lexer->source = strdup(source);
    if (!lexer->source) {
        free(lexer);
        return NULL;
    }
    lexer->sourceLength = strlen(source);

    // 初始化位置信息
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->lineStartOffset = 0;

    // 设置文件名
    lexer->filename = filename ? strdup(filename) : NULL;

    // 设置诊断引擎
    lexer->diagnostics = diagnostics;

    // 初始化关键字表（目前使用静态表，不需要动态创建）
    lexer->keywords = NULL;

    // 初始化状态标志
    lexer->inPreprocessor = false;
    lexer->inComment = false;
    lexer->supportUnicode = false;
    lexer->preserveComments = false;

    lexer->privateData = NULL;

    return lexer;
}

/**
 * @brief 从文件创建词法分析器
 */
Lexer* createLexerFromFile(const char* filename, DiagnosticEngine* diagnostics) {
    if (!filename) {
        return NULL;
    }

    // 读取文件内容
    FILE* file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize <= 0) {
        fclose(file);
        return NULL;
    }

    // 分配缓冲区读取文件
    char* source = (char*)malloc(fileSize + 1);
    if (!source) {
        fclose(file);
        return NULL;
    }

    size_t readSize = fread(source, 1, fileSize, file);
    source[readSize] = '\0';
    fclose(file);

    // 创建词法分析器
    Lexer* lexer = createLexer(source, filename, diagnostics);
    // lexer已经复制了source，释放临时缓冲区
    free(source);

    return lexer;
}

/**
 * @brief 销毁词法分析器
 */
void destroyLexer(Lexer* lexer) {
    if (lexer) {
        if (lexer->source) {
            free(lexer->source);
        }
        if (lexer->filename) {
            free(lexer->filename);
        }
        // 关键字表是静态的，不需要释放
        if (lexer->privateData) {
            free(lexer->privateData);
        }
        free(lexer);
    }
}

// ==================== 跳过空白和注释 ====================

/**
 * @brief 跳过空白字符
 */
void lexerSkipWhitespace(Lexer* lexer) {
    while (!lexerIsAtEnd(lexer)) {
        char ch = lexerCurrentChar(lexer);

        // 处理空白字符
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\v' || ch == '\f') {
            lexerAdvance(lexer);
            continue;
        }

        // 处理反斜杠换行（续行）
        if (ch == '\\' && lexerPeekNext(lexer) == '\n') {
            lexerAdvance(lexer);  // 跳过反斜杠
            lexerAdvance(lexer);  // 跳过换行
            continue;
        }

        break;
    }
}

/**
 * @brief 跳过单行注释
 */
static void lexerSkipLineComment(Lexer* lexer) {
    // 跳过 //
    lexerAdvance(lexer);
    lexerAdvance(lexer);

    // 跳过直到行尾
    while (!lexerIsAtEnd(lexer) && lexerCurrentChar(lexer) != '\n') {
        lexerAdvance(lexer);
    }
}

/**
 * @brief 跳过多行注释
 * @return 成功返回true，遇到未闭合的注释返回false
 */
static bool lexerSkipBlockComment(Lexer* lexer) {
    // 跳过 /*
    size_t startOffset = lexer->position;
    size_t startLine = lexer->line;
    size_t startColumn = lexer->column;
    lexerAdvance(lexer);
    lexerAdvance(lexer);

    while (!lexerIsAtEnd(lexer)) {
        char ch = lexerCurrentChar(lexer);

        // 检查注释结束
        if (ch == '*' && lexerPeekNext(lexer) == '/') {
            lexerAdvance(lexer);  // 跳过 *
            lexerAdvance(lexer);  // 跳过 /
            return true;
        }

        lexerAdvance(lexer);
    }

    // 注释未闭合 - 报告错误
    SourceLocation location = createSourceLocation(
        lexer->filename,
        (int)startLine,
        (int)startColumn,
        (int)startOffset
    );
    lexerReportError(lexer, LEX_ERROR_UNTERMINATED_COMMENT, location,
                    "unterminated block comment");

    return false;
}

/**
 * @brief 跳过注释
 */
bool lexerSkipComment(Lexer* lexer) {
    if (lexerIsAtEnd(lexer)) {
        return true;
    }

    char ch = lexerCurrentChar(lexer);
    char next = lexerPeekNext(lexer);

    // 检查单行注释 //
    if (ch == '/' && next == '/') {
        lexerSkipLineComment(lexer);
        return true;
    }

    // 检查多行注释
    if (ch == '/' && next == '*') {
        return lexerSkipBlockComment(lexer);
    }

    return false;
}

// ==================== 标识符和关键字识别 ====================

/**
 * @brief 检查字符串是否为关键字
 */
TokenType lexerIsKeyword(const Lexer* lexer, const char* str) {
    if (!str) {
        return TOKEN_IDENTIFIER;
    }

    // 线性搜索关键字表
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        if (strcmp(keywordTable[i].keyword, str) == 0) {
            return keywordTable[i].tokenType;
        }
    }

    return TOKEN_IDENTIFIER;
}

/**
 * @brief 读取标识符
 */
static Token lexerReadIdentifier(Lexer* lexer) {
    size_t start = lexer->position;
    size_t startColumn = lexer->column;
    size_t startLine = lexer->line;
    size_t startOffset = lexer->position;

    // 标识符以字母或下划线开头
    char ch = lexerCurrentChar(lexer);
    if (isalpha(ch) || ch == '_') {
        lexerAdvance(lexer);
    }

    // 后续字符可以是字母、数字或下划线
    while (!lexerIsAtEnd(lexer)) {
        ch = lexerCurrentChar(lexer);
        if (isalnum(ch) || ch == '_') {
            lexerAdvance(lexer);
        } else {
            break;
        }
    }

    // 提取标识符字符串
    size_t length = lexer->position - start;
    char* lexeme = (char*)malloc(length + 1);
    if (!lexeme) {
        return *createEOFToken(lexerCreateCurrentLocation(lexer));
    }
    memcpy(lexeme, lexer->source + start, length);
    lexeme[length] = '\0';

    // 检查是否为关键字
    TokenType type = lexerIsKeyword(lexer, lexeme);

    // 创建源位置
    SourceLocation location = createSourceLocation(
        lexer->filename,
        (int)startLine,
        (int)startColumn,
        (int)startOffset
    );

    Token* token = createToken(type, lexeme, location);
    free(lexeme);

    if (token) {
        return *token;
    }
    return *createEOFToken(location);
}

// ==================== 预处理指令识别 ====================

/**
 * @brief 检查字符串是否为预处理指令
 */
static TokenType lexerIsPreprocessorDirective(const char* str) {
    if (!str) {
        return TOKEN_IDENTIFIER;
    }

    // 线性搜索预处理指令表
    for (size_t i = 0; i < PREPROCESSOR_DIRECTIVE_COUNT; i++) {
        if (strcmp(preprocessorDirectiveTable[i].directive, str) == 0) {
            return preprocessorDirectiveTable[i].tokenType;
        }
    }

    return TOKEN_IDENTIFIER;
}

/**
 * @brief 读取预处理指令
 */
static Token lexerReadPreprocessorDirective(Lexer* lexer) {
    size_t startColumn = lexer->column;
    size_t startLine = lexer->line;
    size_t startOffset = lexer->position;

    // 跳过 #
    lexerAdvance(lexer);

    // 跳过 # 后的空白
    lexerSkipWhitespace(lexer);

    // 读取指令名称
    size_t directiveStart = lexer->position;
    while (!lexerIsAtEnd(lexer) && isalpha(lexerCurrentChar(lexer))) {
        lexerAdvance(lexer);
    }

    // 提取指令名称
    size_t directiveLength = lexer->position - directiveStart;
    char* directive = (char*)malloc(directiveLength + 1);
    if (!directive) {
        return *createEOFToken(lexerCreateCurrentLocation(lexer));
    }
    memcpy(directive, lexer->source + directiveStart, directiveLength);
    directive[directiveLength] = '\0';

    // 检查是否为已知预处理指令
    TokenType type = lexerIsPreprocessorDirective(directive);

    // 创建源位置
    SourceLocation location = createSourceLocation(
        lexer->filename,
        (int)startLine,
        (int)startColumn,
        (int)startOffset
    );

    // 读取整行作为词素
    while (!lexerIsAtEnd(lexer) && lexerCurrentChar(lexer) != '\n') {
        lexerAdvance(lexer);
    }

    size_t lexemeLength = lexer->position - startOffset;
    char* lexeme = (char*)malloc(lexemeLength + 1);
    if (lexeme) {
        memcpy(lexeme, lexer->source + startOffset, lexemeLength);
        lexeme[lexemeLength] = '\0';
    }

    Token* token = createToken(type, lexeme ? lexeme : directive, location);
    free(directive);
    if (lexeme) free(lexeme);

    if (token) {
        return *token;
    }
    return *createEOFToken(location);
}

// ==================== 数字字面量识别 ====================

/**
 * @brief 读取数字前缀（0x, 0b, 0等）
 */
static int lexerDetectNumberBase(Lexer* lexer) {
    char ch = lexerCurrentChar(lexer);
    char next = lexerPeekNext(lexer);

    // 检查十六进制前缀 0x 或 0X
    if (ch == '0' && (next == 'x' || next == 'X')) {
        lexerAdvance(lexer);  // 跳过 0
        lexerAdvance(lexer);  // 跳过 x/X
        return 16;
    }

    // 检查二进制前缀 0b 或 0B (C23)
    if (ch == '0' && (next == 'b' || next == 'B')) {
        lexerAdvance(lexer);  // 跳过 0
        lexerAdvance(lexer);  // 跳过 b/B
        return 2;
    }

    // 检查八进制前缀 0
    if (ch == '0' && isdigit(next)) {
        lexerAdvance(lexer);  // 跳过 0
        return 8;
    }

    return 10;  // 默认十进制
}

/**
 * @brief 读取整数字面量
 */
static Token lexerReadInteger(Lexer* lexer, int base) {
    size_t start = lexer->position;
    size_t startColumn = lexer->column;
    size_t startLine = lexer->line;
    size_t startOffset = lexer->position;

    // 读取数字
    while (!lexerIsAtEnd(lexer)) {
        char ch = lexerCurrentChar(lexer);

        // 根据进制判断有效字符
        bool valid = false;
        if (base == 16) {
            valid = isxdigit(ch);
        } else if (base == 10) {
            valid = isdigit(ch);
        } else if (base == 8) {
            valid = (ch >= '0' && ch <= '7');
        } else if (base == 2) {
            valid = (ch == '0' || ch == '1');
        }

        if (!valid) {
            break;
        }
        lexerAdvance(lexer);
    }

    // 检查类型后缀 (u, l, ll, ul, ull 等)
    while (!lexerIsAtEnd(lexer)) {
        char ch = tolower(lexerCurrentChar(lexer));
        if (ch == 'u' || ch == 'l') {
            lexerAdvance(lexer);
        } else {
            break;
        }
    }

    // 提取数字字符串
    size_t length = lexer->position - start;
    char* lexeme = (char*)malloc(length + 1);
    if (!lexeme) {
        return *createEOFToken(lexerCreateCurrentLocation(lexer));
    }
    memcpy(lexeme, lexer->source + start, length);
    lexeme[length] = '\0';

    // 创建源位置
    SourceLocation location = createSourceLocation(
        lexer->filename,
        (int)startLine,
        (int)startColumn,
        (int)startOffset
    );

    Token* token = createIntegerToken(lexeme, location, base);
    free(lexeme);

    if (token) {
        return *token;
    }
    return *createEOFToken(location);
}

/**
 * @brief 读取浮点数字面量
 */
static Token lexerReadFloat(Lexer* lexer) {
    size_t start = lexer->position;
    size_t startColumn = lexer->column;
    size_t startLine = lexer->line;
    size_t startOffset = lexer->position;

    // 读取整数部分
    while (!lexerIsAtEnd(lexer) && isdigit(lexerCurrentChar(lexer))) {
        lexerAdvance(lexer);
    }

    // 读取小数点
    if (lexerCurrentChar(lexer) == '.') {
        lexerAdvance(lexer);

        // 读取小数部分
        while (!lexerIsAtEnd(lexer) && isdigit(lexerCurrentChar(lexer))) {
            lexerAdvance(lexer);
        }
    }

    // 读取指数部分
    char ch = lexerCurrentChar(lexer);
    if (ch == 'e' || ch == 'E') {
        lexerAdvance(lexer);

        // 检查可选的正负号
        if (lexerCurrentChar(lexer) == '+' || lexerCurrentChar(lexer) == '-') {
            lexerAdvance(lexer);
        }

        // 读取指数数字
        while (!lexerIsAtEnd(lexer) && isdigit(lexerCurrentChar(lexer))) {
            lexerAdvance(lexer);
        }
    }

    // 检查类型后缀 (f, l, F, L)
    ch = tolower(lexerCurrentChar(lexer));
    if (ch == 'f' || ch == 'l') {
        lexerAdvance(lexer);
    }

    // 提取数字字符串
    size_t length = lexer->position - start;
    char* lexeme = (char*)malloc(length + 1);
    if (!lexeme) {
        return *createEOFToken(lexerCreateCurrentLocation(lexer));
    }
    memcpy(lexeme, lexer->source + start, length);
    lexeme[length] = '\0';

    // 创建源位置
    SourceLocation location = createSourceLocation(
        lexer->filename,
        (int)startLine,
        (int)startColumn,
        (int)startOffset
    );

    Token* token = createFloatToken(lexeme, location);
    free(lexeme);

    if (token) {
        return *token;
    }
    return *createEOFToken(location);
}

/**
 * @brief 读取数字字面量
 */
static Token lexerReadNumber(Lexer* lexer) {
    // 检测进制
    int base = lexerDetectNumberBase(lexer);

    // 回退一位，重新检查
    if (base != 10) {
        size_t oldPos = lexer->position;
        size_t oldCol = lexer->column;
        size_t oldLine = lexer->line;

        // 尝试读取为整数
        Token token = lexerReadInteger(lexer, base);

        // 检查是否有小数点（可能是浮点数）
        if (lexerCurrentChar(lexer) == '.') {
            // 回退，重新解析为浮点数
            lexer->position = oldPos - 2;
            lexer->column = oldCol;
            lexer->line = oldLine;
            return lexerReadFloat(lexer);
        }

        return token;
    }

    // 十进制数可能是整数或浮点数
    // 向前查看判断是否有小数点或科学计数法
    size_t lookahead = lexer->position;
    bool isFloat = false;

    while (lookahead < lexer->sourceLength && isdigit(lexer->source[lookahead])) {
        lookahead++;
    }

    if (lookahead < lexer->sourceLength) {
        char ch = lexer->source[lookahead];
        if (ch == '.' || ch == 'e' || ch == 'E') {
            isFloat = true;
        }
    }

    if (isFloat) {
        return lexerReadFloat(lexer);
    }

    return lexerReadInteger(lexer, 10);
}

// ==================== 字符和字符串字面量识别 ====================

/**
 * @brief 处理转义序列
 */
static char lexerProcessEscapeSequence(Lexer* lexer, bool* success) {
    *success = true;
    lexerAdvance(lexer);  // 跳过反斜杠

    if (lexerIsAtEnd(lexer)) {
        *success = false;
        return '\\';
    }

    char ch = lexerCurrentChar(lexer);

    switch (ch) {
        case 'n':  lexerAdvance(lexer); return '\n';
        case 't':  lexerAdvance(lexer); return '\t';
        case 'r':  lexerAdvance(lexer); return '\r';
        case 'b':  lexerAdvance(lexer); return '\b';
        case 'f':  lexerAdvance(lexer); return '\f';
        case 'v':  lexerAdvance(lexer); return '\v';
        case 'a':  lexerAdvance(lexer); return '\a';
        case '\\': lexerAdvance(lexer); return '\\';
        case '?':  lexerAdvance(lexer); return '\?';
        case '\'': lexerAdvance(lexer); return '\'';
        case '"':  lexerAdvance(lexer); return '"';

        // 八进制转义 \ddd
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': {
            int value = 0;
            int count = 0;
            while (count < 3 && !lexerIsAtEnd(lexer)) {
                ch = lexerCurrentChar(lexer);
                if (ch >= '0' && ch <= '7') {
                    value = value * 8 + (ch - '0');
                    lexerAdvance(lexer);
                    count++;
                } else {
                    break;
                }
            }
            return (char)value;
        }

        // 十六进制转义 \xhh
        case 'x':
        case 'X': {
            lexerAdvance(lexer);  // 跳过 x
            int value = 0;
            int count = 0;
            while (count < 2 && !lexerIsAtEnd(lexer)) {
                ch = lexerCurrentChar(lexer);
                if (isxdigit(ch)) {
                    value = value * 16 + (isdigit(ch) ? (ch - '0') : (tolower(ch) - 'a' + 10));
                    lexerAdvance(lexer);
                    count++;
                } else {
                    break;
                }
            }
            return (char)value;
        }

        // Unicode转义 \uhhhh 或 \Uhhhhhhhh
        case 'u':
        case 'U': {
            // 简化处理，跳过Unicode转义
            lexerAdvance(lexer);
            int maxDigits = (ch == 'u') ? 4 : 8;
            for (int i = 0; i < maxDigits && !lexerIsAtEnd(lexer); i++) {
                if (isxdigit(lexerCurrentChar(lexer))) {
                    lexerAdvance(lexer);
                }
            }
            return '?';  // 占位符
        }

        default:
            *success = false;
            lexerAdvance(lexer);
            return ch;
    }
}

/**
 * @brief 读取字符字面量
 */
static Token lexerReadChar(Lexer* lexer) {
    size_t startColumn = lexer->column;
    size_t startLine = lexer->line;
    size_t startOffset = lexer->position;
    bool isWide = false;

    // 检查宽字符前缀 L
    if (lexerCurrentChar(lexer) == 'L') {
        isWide = true;
        lexerAdvance(lexer);
    }

    // 跳过开引号
    lexerAdvance(lexer);

    if (lexerIsAtEnd(lexer)) {
        SourceLocation loc = createSourceLocation(
            lexer->filename,
            (int)startLine,
            (int)startColumn,
            (int)startOffset
        );
        return *createEOFToken(loc);
    }

    // 读取字符内容
    char value = '\0';
    bool success = true;

    if (lexerCurrentChar(lexer) == '\\') {
        value = lexerProcessEscapeSequence(lexer, &success);
    } else {
        value = lexerCurrentChar(lexer);
        lexerAdvance(lexer);
    }

    // 跳过闭引号
    if (!lexerIsAtEnd(lexer) && lexerCurrentChar(lexer) == '\'') {
        lexerAdvance(lexer);
    }

    // 创建源位置
    SourceLocation location = createSourceLocation(
        lexer->filename,
        (int)startLine,
        (int)startColumn,
        (int)startOffset
    );

    // 构造词素
    char lexeme[4] = { '\0', '\0', '\0', '\0' };
    lexeme[0] = '\'';
    if (isWide) {
        lexeme[0] = 'L';
        lexeme[1] = '\'';
        lexeme[2] = value;
        lexeme[3] = '\'';
    } else {
        lexeme[1] = value;
        lexeme[2] = '\'';
    }

    Token* token = createTokenWithCharValue(TOKEN_CHAR_LITERAL, lexeme, location, value, isWide);
    if (token) {
        return *token;
    }
    return *createEOFToken(location);
}

/**
 * @brief 读取字符串字面量
 */
static Token lexerReadString(Lexer* lexer) {
    size_t start = lexer->position;
    size_t startColumn = lexer->column;
    size_t startLine = lexer->line;
    size_t startOffset = lexer->position;
    bool isWide = false;

    // 检查宽字符串前缀 L
    if (lexerCurrentChar(lexer) == 'L') {
        isWide = true;
        lexerAdvance(lexer);
    }

    // 跳过开引号
    lexerAdvance(lexer);

    // 构建字符串内容
    size_t bufferSize = 64;
    size_t length = 0;
    char* buffer = (char*)malloc(bufferSize);
    if (!buffer) {
        return *createEOFToken(lexerCreateCurrentLocation(lexer));
    }

    while (!lexerIsAtEnd(lexer)) {
        char ch = lexerCurrentChar(lexer);

        if (ch == '"') {
            lexerAdvance(lexer);  // 跳过闭引号
            break;
        }

        if (ch == '\n') {
            // 字符串不能跨行（不支持续行）
            break;
        }

        if (ch == '\\') {
            bool success;
            ch = lexerProcessEscapeSequence(lexer, &success);
        } else {
            lexerAdvance(lexer);
        }

        // 扩展缓冲区
        if (length >= bufferSize - 1) {
            bufferSize *= 2;
            char* newBuffer = (char*)realloc(buffer, bufferSize);
            if (!newBuffer) {
                free(buffer);
                return *createEOFToken(lexerCreateCurrentLocation(lexer));
            }
            buffer = newBuffer;
        }

        buffer[length++] = ch;
    }

    buffer[length] = '\0';

    // 提取词素
    size_t lexemeLength = lexer->position - start;
    char* lexeme = (char*)malloc(lexemeLength + 1);
    if (lexeme) {
        memcpy(lexeme, lexer->source + start, lexemeLength);
        lexeme[lexemeLength] = '\0';
    }

    // 创建源位置
    SourceLocation location = createSourceLocation(
        lexer->filename,
        (int)startLine,
        (int)startColumn,
        (int)startOffset
    );

    Token* token = createTokenWithStringValue(TOKEN_STRING_LITERAL, lexeme ? lexeme : "", location, buffer, isWide);
    free(buffer);
    if (lexeme) free(lexeme);

    if (token) {
        return *token;
    }
    return *createEOFToken(location);
}

// ==================== 运算符和分隔符识别 ====================

/**
 * @brief 读取运算符或分隔符
 */
static Token lexerReadOperatorOrPunctuation(Lexer* lexer) {
    char ch = lexerCurrentChar(lexer);
    size_t startColumn = lexer->column;
    size_t startLine = lexer->line;
    size_t startOffset = lexer->position;

    SourceLocation location = createSourceLocation(
        lexer->filename,
        (int)startLine,
        (int)startColumn,
        (int)startOffset
    );

    // 处理多字符运算符
    char next = lexerPeekNext(lexer);

    switch (ch) {
        // 赋值运算符
        case '=':
            lexerAdvance(lexer);
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_EQUAL, location);
            }
            return *createOperatorToken(TOKEN_ASSIGN, location);

        case '!':
            lexerAdvance(lexer);
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_NOT_EQUAL, location);
            }
            return *createOperatorToken(TOKEN_LOGICAL_NOT, location);

        case '<':
            lexerAdvance(lexer);
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_LESS_EQUAL, location);
            }
            if (next == '<') {
                lexerAdvance(lexer);
                if (lexerPeekNext(lexer) == '=') {
                    lexerAdvance(lexer);
                    return *createOperatorToken(TOKEN_LEFT_SHIFT, location);  // <<= 需要后续处理
                }
                return *createOperatorToken(TOKEN_LEFT_SHIFT, location);
            }
            return *createOperatorToken(TOKEN_LESS, location);

        case '>':
            lexerAdvance(lexer);
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_GREATER_EQUAL, location);
            }
            if (next == '>') {
                lexerAdvance(lexer);
                if (lexerPeekNext(lexer) == '=') {
                    lexerAdvance(lexer);
                    return *createOperatorToken(TOKEN_RIGHT_SHIFT, location);  // >>= 需要后续处理
                }
                return *createOperatorToken(TOKEN_RIGHT_SHIFT, location);
            }
            return *createOperatorToken(TOKEN_GREATER, location);

        case '&':
            lexerAdvance(lexer);
            if (next == '&') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_LOGICAL_AND, location);
            }
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_BITWISE_AND, location);  // &= 需要后续处理
            }
            return *createOperatorToken(TOKEN_BITWISE_AND, location);

        case '|':
            lexerAdvance(lexer);
            if (next == '|') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_LOGICAL_OR, location);
            }
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_BITWISE_OR, location);  // |= 需要后续处理
            }
            return *createOperatorToken(TOKEN_BITWISE_OR, location);

        case '^':
            lexerAdvance(lexer);
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_BITWISE_XOR, location);  // ^= 需要后续处理
            }
            return *createOperatorToken(TOKEN_BITWISE_XOR, location);

        case '+':
            lexerAdvance(lexer);
            if (next == '+') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_INCREMENT, location);
            }
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_PLUS_ASSIGN, location);
            }
            return *createOperatorToken(TOKEN_PLUS, location);

        case '-':
            lexerAdvance(lexer);
            if (next == '-') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_DECREMENT, location);
            }
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_MINUS_ASSIGN, location);
            }
            if (next == '>') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_ARROW, location);
            }
            return *createOperatorToken(TOKEN_MINUS, location);

        case '*':
            lexerAdvance(lexer);
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_MULTIPLY_ASSIGN, location);
            }
            return *createOperatorToken(TOKEN_MULTIPLY, location);

        case '/':
            lexerAdvance(lexer);
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_DIVIDE_ASSIGN, location);
            }
            return *createOperatorToken(TOKEN_DIVIDE, location);

        case '%':
            lexerAdvance(lexer);
            if (next == '=') {
                lexerAdvance(lexer);
                return *createOperatorToken(TOKEN_MODULO_ASSIGN, location);
            }
            return *createOperatorToken(TOKEN_MODULO, location);

        // 分隔符
        case '(':
            lexerAdvance(lexer);
            return *createPunctuationToken(TOKEN_LPAREN, location);

        case ')':
            lexerAdvance(lexer);
            return *createPunctuationToken(TOKEN_RPAREN, location);

        case '[':
            lexerAdvance(lexer);
            return *createPunctuationToken(TOKEN_LBRACKET, location);

        case ']':
            lexerAdvance(lexer);
            return *createPunctuationToken(TOKEN_RBRACKET, location);

        case '{':
            lexerAdvance(lexer);
            return *createToken(TOKEN_LBRACE, "{", location);

        case '}':
            lexerAdvance(lexer);
            return *createToken(TOKEN_RBRACE, "}", location);

        case ';':
            lexerAdvance(lexer);
            return *createPunctuationToken(TOKEN_SEMICOLON, location);

        case ',':
            lexerAdvance(lexer);
            return *createPunctuationToken(TOKEN_COMMA, location);

        case '.':
            lexerAdvance(lexer);
            if (next == '.') {
                lexerAdvance(lexer);
                if (lexerPeekNext(lexer) == '.') {
                    lexerAdvance(lexer);
                    return *createPunctuationToken(TOKEN_ELLIPSSIS, location);
                }
            }
            return *createPunctuationToken(TOKEN_DOT, location);

        case ':':
            lexerAdvance(lexer);
            return *createPunctuationToken(TOKEN_COLON, location);

        case '?':
            lexerAdvance(lexer);
            return *createPunctuationToken(TOKEN_QUESTION, location);

        case '~':
            lexerAdvance(lexer);
            return *createOperatorToken(TOKEN_BITWISE_NOT, location);

        default:
            // 未知字符
            lexerAdvance(lexer);
            return *createToken(TOKEN_UNKNOWN, NULL, location);
    }
}

// ==================== 主词法分析函数 ====================

/**
 * @brief 获取下一个token
 */
Token lexerNextToken(Lexer* lexer) {
    // 跳过空白字符
    lexerSkipWhitespace(lexer);

    // 跳过注释
    while (lexerSkipComment(lexer)) {
        lexerSkipWhitespace(lexer);
    }

    // 检查文件结束
    if (lexerIsAtEnd(lexer)) {
        return *createEOFToken(lexerCreateCurrentLocation(lexer));
    }

    char ch = lexerCurrentChar(lexer);

    // 预处理指令
    if (ch == '#') {
        return lexerReadPreprocessorDirective(lexer);
    }

    // 标识符或关键字
    if (isalpha(ch) || ch == '_') {
        return lexerReadIdentifier(lexer);
    }

    // 数字字面量
    if (isdigit(ch)) {
        return lexerReadNumber(lexer);
    }

    // 字符字面量
    if (ch == '\'' || (ch == 'L' && lexerPeekNext(lexer) == '\'')) {
        return lexerReadChar(lexer);
    }

    // 字符串字面量
    if (ch == '"' || (ch == 'L' && lexerPeekNext(lexer) == '"')) {
        return lexerReadString(lexer);
    }

    // 运算符和分隔符
    if (ispunct(ch)) {
        return lexerReadOperatorOrPunctuation(lexer);
    }

    // 未知字符
    SourceLocation location = lexerCreateCurrentLocation(lexer);
    lexerAdvance(lexer);
    return *createToken(TOKEN_UNKNOWN, NULL, location);
}

/**
 * @brief 查看下一个token但不移动位置
 */
Token lexerPeekToken(Lexer* lexer) {
    // 保存当前状态
    size_t savedPosition = lexer->position;
    size_t savedLine = lexer->line;
    size_t savedColumn = lexer->column;
    size_t savedLineStartOffset = lexer->lineStartOffset;

    // 获取下一个token
    Token token = lexerNextToken(lexer);

    // 恢复状态
    lexer->position = savedPosition;
    lexer->line = savedLine;
    lexer->column = savedColumn;
    lexer->lineStartOffset = savedLineStartOffset;

    return token;
}

/**
 * @brief 对整个源代码进行词法分析
 * @param lexer 词法分析器
 * @return token向量（Vector<Token>），失败返回NULL
 */
Vector* lexerTokenize(Lexer* lexer) {
    if (!lexer) {
        return NULL;
    }

    // 创建Vector来存储Token
    Vector* tokens = vectorCreate(sizeof(Token), 16);
    if (!tokens) {
        return NULL;
    }

    // 循环获取所有token
    while (true) {
        Token token = lexerNextToken(lexer);

        // 将token添加到Vector
        if (!vectorPushBack(tokens, &token)) {
            // 添加失败，清理已创建的token
            vectorDestroy(tokens, (void (*)(void*))destroyToken);
            return NULL;
        }

        // 检查是否到达文件末尾
        if (token.type == TOKEN_EOF) {
            break;
        }
    }

    return tokens;
}

// ==================== 状态管理 ====================

/**
 * @brief 重置词法分析器到初始状态
 */
void lexerReset(Lexer* lexer) {
    if (lexer) {
        lexer->position = 0;
        lexer->line = 1;
        lexer->column = 1;
        lexer->lineStartOffset = 0;
        lexer->inPreprocessor = false;
        lexer->inComment = false;
    }
}

/**
 * 
 * @brief 获取当前位置
 */
void lexerGetPosition(const Lexer* lexer, size_t* line, size_t* column) {
    if (lexer) {
        if (line) *line = lexer->line;
        if (column) *column = lexer->column;
    }
}

/**
 * @brief 设置是否保留注释token
 */
void lexerSetPreserveComments(Lexer* lexer, bool preserve) {
    if (lexer) {
        lexer->preserveComments = preserve;
    }
}

/**
 * @brief 设置是否支持Unicode
 */
void lexerSetSupportUnicode(Lexer* lexer, bool support) {
    if (lexer) {
        lexer->supportUnicode = support;
    }
}

/**
 * @brief 获取当前源位置
 */
SourceLocation lexerGetCurrentLocation(const Lexer* lexer) {
    if (lexer) {
        return lexerCreateCurrentLocation(lexer);
    }
    return createSourceLocation(NULL, 0, 0, 0);
}
