#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdbool.h>
#include <stddef.h>
#include "../../common/diagnostics/source_location.h"
#include "../../common/containers/vector.h"

/**
 * @brief 错误严重程度
 */
typedef enum {
    ERROR_LEVEL_WARNING,    // 警告
    ERROR_LEVEL_ERROR,      // 错误
    ERROR_LEVEL_FATAL       // 致命错误
} ErrorLevel;

/**
 * @brief 错误类型
 */
typedef enum {
    ERROR_TYPE_LEXICAL,     // 词法错误
    ERROR_TYPE_SYNTAX,      // 语法错误
    ERROR_TYPE_SEMANTIC,    // 语义错误
    ERROR_TYPE_IO,          // IO错误
    ERROR_TYPE_MEMORY       // 内存错误
} ErrorType;

/**
 * @brief 错误信息结构体
 */
typedef struct {
    ErrorLevel level;           // 错误级别
    ErrorType type;             // 错误类型
    SourceLocation location;     // 错误位置
    char* message;              // 错误消息
    char* suggestion;           // 修复建议（可选）
    int errorCode;              // 错误代码
} ErrorInfo;

/**
 * @brief 错误处理器结构体
 */
typedef struct {
    Vector* errors;             // 错误列表
    bool hasError;              // 是否有错误
    bool hasWarning;            // 是否有警告
    int errorCount;             // 错误计数
    int warningCount;           // 警告计数
    bool verbose;               // 是否显示详细信息
    bool stopOnError;           // 遇到错误是否停止
} ErrorHandler;

// ==================== 构造函数和析构函数 ====================

/**
 * @brief 创建错误处理器
 * @return 新创建的错误处理器，失败返回NULL
 */
ErrorHandler* createErrorHandler(void);

/**
 * @brief 销毁错误处理器
 * @param handler 要销毁的错误处理器
 */
void destroyErrorHandler(ErrorHandler* handler);

// ==================== 错误报告 ====================

/**
 * @brief 报告错误
 * @param handler 错误处理器
 * @param level 错误级别
 * @param type 错误类型
 * @param location 错误位置
 * @param message 错误消息
 * @param suggestion 修复建议（可为NULL）
 * @return 成功返回true，失败返回false
 */
bool reportError(ErrorHandler* handler, ErrorLevel level, ErrorType type,
                SourceLocation location, const char* message, const char* suggestion);

/**
 * @brief 报告词法错误
 * @param handler 错误处理器
 * @param location 错误位置
 * @param message 错误消息
 * @return 成功返回true，失败返回false
 */
bool reportLexicalError(ErrorHandler* handler, SourceLocation location, const char* message);

/**
 * @brief 报告警告
 * @param handler 错误处理器
 * @param location 警告位置
 * @param message 警告消息
 * @return 成功返回true，失败返回false
 */
bool reportWarning(ErrorHandler* handler, SourceLocation location, const char* message);

/**
 * @brief 报告致命错误
 * @param handler 错误处理器
 * @param location 错误位置
 * @param message 错误消息
 * @return 成功返回true，失败返回false
 */
bool reportFatalError(ErrorHandler* handler, SourceLocation location, const char* message);

// ==================== 错误查询 ====================

/**
 * @brief 获取错误数量
 * @param handler 错误处理器
 * @return 错误数量
 */
int getErrorCount(const ErrorHandler* handler);

/**
 * @brief 获取警告数量
 * @param handler 错误处理器
 * @return 警告数量
 */
int getWarningCount(const ErrorHandler* handler);

/**
 * @brief 检查是否有错误
 * @param handler 错误处理器
 * @return 有错误返回true，否则返回false
 */
bool hasErrors(const ErrorHandler* handler);

/**
 * @brief 检查是否有警告
 * @param handler 错误处理器
 * @return 有警告返回true，否则返回false
 */
bool hasWarnings(const ErrorHandler* handler);

// ==================== 错误输出 ====================

/**
 * @brief 打印所有错误
 * @param handler 错误处理器
 */
void printErrors(const ErrorHandler* handler);

/**
 * @brief 打印错误摘要
 * @param handler 错误处理器
 */
void printErrorSummary(const ErrorHandler* handler);

/**
 * @brief 清空所有错误
 * @param handler 错误处理器
 */
void clearErrors(ErrorHandler* handler);

// ==================== 错误处理辅助函数 ====================

/**
 * @brief 格式化错误消息
 * @param error 错误信息
 * @return 格式化后的字符串（需要调用者释放）
 */
char* formatErrorMessage(const ErrorInfo* error);

/**
 * @brief 获取错误级别字符串
 * @param level 错误级别
 * @return 错误级别字符串
 */
const char* errorLevelToString(ErrorLevel level);

/**
 * @brief 获取错误类型字符串
 * @param type 错误类型
 * @return 错误类型字符串
 */
const char* errorTypeToString(ErrorType type);

#endif // ERROR_HANDLER_H
