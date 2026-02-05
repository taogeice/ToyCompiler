#ifndef DIAGNOSTIC_ENGINE_H
#define DIAGNOSTIC_ENGINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include "source_location.h"

// 前向声明
typedef struct DiagnosticConsumer DiagnosticConsumer;
typedef struct DiagnosticEngine DiagnosticEngine;

/**
 * @brief 诊断级别
 */
typedef enum {
    DIAGNOSTIC_LEVEL_NOTE,      // 备注
    DIAGNOSTIC_LEVEL_WARNING,   // 警告
    DIAGNOSTIC_LEVEL_ERROR,     // 错误
    DIAGNOSTIC_LEVEL_FATAL      // 致命错误
} DiagnosticLevel;

// 兼容宏
#define DIAGNOSTIC_ERROR DIAGNOSTIC_LEVEL_ERROR
#define DIAGNOSTIC_WARNING DIAGNOSTIC_LEVEL_WARNING
#define DIAGNOSTIC_NOTE DIAGNOSTIC_LEVEL_NOTE
#define DIAGNOSTIC_FATAL DIAGNOSTIC_LEVEL_FATAL

/**
 * @brief 诊断消息结构体
 */
typedef struct {
    DiagnosticLevel level;
    SourceLocation location;
    char* message;
    char* category;      // 错误类别（如 "lexer", "parser", "type" 等）
    bool isError;
} Diagnostic;

/**
 * @brief 诊断消费者接口
 * 用于接收和处理诊断消息
 */
typedef struct DiagnosticConsumer {
    void (*handleDiagnostic)(struct DiagnosticConsumer* consumer, const Diagnostic* diagnostic);
    void (*destroy)(struct DiagnosticConsumer* consumer);
    void* privateData;
} DiagnosticConsumer;

/**
 * @brief 诊断引擎结构体
 */
struct DiagnosticEngine {
    DiagnosticConsumer* consumer;
    size_t errorCount;
    size_t warningCount;
    bool suppressErrors;
    bool suppressWarnings;
    bool fatalErrorOccurred;
    void* privateData;
};

// ==================== 构造函数和析构函数 ====================

/**
 * @brief 创建诊断引擎
 * @param consumer 诊断消费者（可为NULL，使用默认消费者）
 * @return 新创建的诊断引擎，失败返回NULL
 */
DiagnosticEngine* createDiagnosticEngine(DiagnosticConsumer* consumer);

/**
 * @brief 销毁诊断引擎
 * @param engine 要销毁的诊断引擎
 */
void destroyDiagnosticEngine(DiagnosticEngine* engine);

// ==================== 诊断报告 ====================

/**
 * @brief 报告诊断消息
 * @param engine 诊断引擎
 * @param level 诊断级别
 * @param location 源位置
 * @param format 消息格式字符串
 * @param ... 格式化参数
 */
void diagnosticEngineReport(DiagnosticEngine* engine,
                           DiagnosticLevel level,
                           SourceLocation location,
                           const char* format,
                           ...);

/**
 * @brief 报告错误
 */
static inline void diagnosticEngineError(DiagnosticEngine* engine,
                                       SourceLocation location,
                                       const char* format, ...) {
    va_list args;
    va_start(args, format);
    // diagnosticEngineReport实现需要处理可变参数
    va_end(args);
}

/**
 * @brief 报告警告
 */
static inline void diagnosticEngineWarning(DiagnosticEngine* engine,
                                         SourceLocation location,
                                         const char* format, ...) {
    va_list args;
    va_start(args, format);
    va_end(args);
}

/**
 * @brief 报告备注
 */
static inline void diagnosticEngineNote(DiagnosticEngine* engine,
                                      SourceLocation location,
                                      const char* format, ...) {
    va_list args;
    va_start(args, format);
    va_end(args);
}

// ==================== 状态管理 ====================

/**
 * @brief 获取错误计数
 */
size_t diagnosticEngineGetErrorCount(const DiagnosticEngine* engine);

/**
 * @brief 获取警告计数
 */
size_t diagnosticEngineGetWarningCount(const DiagnosticEngine* engine);

/**
 * @brief 是否有错误发生
 */
bool diagnosticEngineHasErrors(const DiagnosticEngine* engine);

/**
 * @brief 重置计数器
 */
void diagnosticEngineResetCounters(DiagnosticEngine* engine);

/**
 * @brief 设置是否抑制错误
 */
void diagnosticEngineSetSuppressErrors(DiagnosticEngine* engine, bool suppress);

/**
 * @brief 设置是否抑制警告
 */
void diagnosticEngineSetSuppressWarnings(DiagnosticEngine* engine, bool suppress);

// ==================== 诊断消费者 ====================

/**
 * @brief 创建标准错误输出消费者
 */
DiagnosticConsumer* createStderrDiagnosticConsumer(void);

/**
 * @brief 创建文件输出消费者
 */
DiagnosticConsumer* createFileDiagnosticConsumer(FILE* output);

/**
 * @brief 创建内存缓冲消费者（用于测试）
 */
DiagnosticConsumer* createBufferDiagnosticConsumer(char* buffer, size_t bufferSize);

/**
 * @brief 销毁诊断消费者
 */
void destroyDiagnosticConsumer(DiagnosticConsumer* consumer);

// ==================== 辅助函数 ====================

/**
 * @brief 获取诊断级别名称
 */
const char* diagnosticLevelToString(DiagnosticLevel level);

/**
 * @brief 格式化诊断消息
 */
char* formatDiagnostic(const Diagnostic* diagnostic);

#endif // DIAGNOSTIC_ENGINE_H
