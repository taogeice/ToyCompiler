#include "diagnostic_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// ==================== 内部辅助函数 ====================

/**
 * @brief 获取诊断级别名称
 */
const char* diagnosticLevelToString(DiagnosticLevel level) {
    switch (level) {
        case DIAGNOSTIC_LEVEL_NOTE:    return "note";
        case DIAGNOSTIC_LEVEL_WARNING: return "warning";
        case DIAGNOSTIC_LEVEL_ERROR:   return "error";
        case DIAGNOSTIC_LEVEL_FATAL:   return "fatal error";
        default:                       return "unknown";
    }
}

/**
 * @brief 格式化诊断消息
 */
char* formatDiagnostic(const Diagnostic* diagnostic) {
    if (!diagnostic) {
        return NULL;
    }

    // 计算所需缓冲区大小
    size_t bufferSize = 512;
    char* buffer = (char*)malloc(bufferSize);
    if (!buffer) {
        return NULL;
    }

    // 格式化消息
    if (diagnostic->location.filename) {
        snprintf(buffer, bufferSize,
                "%s:%d:%d: %s: %s",
                diagnostic->location.filename,
                diagnostic->location.line,
                diagnostic->location.column,
                diagnosticLevelToString(diagnostic->level),
                diagnostic->message);
    } else {
        snprintf(buffer, bufferSize,
                "%s: %s",
                diagnosticLevelToString(diagnostic->level),
                diagnostic->message);
    }

    return buffer;
}

// ==================== 诊断消费者实现 ====================

/**
 * @brief 标准错误输出消费者
 */
typedef struct {
    DiagnosticConsumer base;
    FILE* output;
    bool colored;
} StderrDiagnosticConsumer;

static void stderrDiagnosticConsumerHandle(DiagnosticConsumer* consumer,
                                          const Diagnostic* diagnostic) {
    if (!consumer || !diagnostic) {
        return;
    }

    StderrDiagnosticConsumer* stderrConsumer = (StderrDiagnosticConsumer*)consumer;
    FILE* output = stderrConsumer->output ? stderrConsumer->output : stderr;

    // ANSI颜色代码
    const char* reset = stderrConsumer->colored ? "\033[0m" : "";
    const char* color = "";
    if (stderrConsumer->colored) {
        switch (diagnostic->level) {
            case DIAGNOSTIC_LEVEL_NOTE:    color = "\033[0;36m"; break;  // Cyan
            case DIAGNOSTIC_LEVEL_WARNING: color = "\033[0;33m"; break;  // Yellow
            case DIAGNOSTIC_LEVEL_ERROR:   color = "\033[0;31m"; break;  // Red
            case DIAGNOSTIC_LEVEL_FATAL:   color = "\033[1;31m"; break;  // Bold Red
        }
    }

    char* formatted = formatDiagnostic(diagnostic);
    if (formatted) {
        fprintf(output, "%s%s%s\n", color, formatted, reset);
        free(formatted);
    }
}

static void stderrDiagnosticConsumerDestroy(DiagnosticConsumer* consumer) {
    if (consumer) {
        free(consumer);
    }
}

DiagnosticConsumer* createStderrDiagnosticConsumer(void) {
    StderrDiagnosticConsumer* consumer = (StderrDiagnosticConsumer*)malloc(sizeof(StderrDiagnosticConsumer));
    if (!consumer) {
        return NULL;
    }

    consumer->base.handleDiagnostic = stderrDiagnosticConsumerHandle;
    consumer->base.destroy = stderrDiagnosticConsumerDestroy;
    consumer->base.privateData = NULL;
    consumer->output = stderr;
    consumer->colored = true;  // 默认启用颜色

    return (DiagnosticConsumer*)consumer;
}

/**
 * @brief 文件输出消费者
 */
typedef struct {
    DiagnosticConsumer base;
    FILE* output;
    bool shouldClose;
} FileDiagnosticConsumer;

static void fileDiagnosticConsumerHandle(DiagnosticConsumer* consumer,
                                        const Diagnostic* diagnostic) {
    if (!consumer || !diagnostic) {
        return;
    }

    FileDiagnosticConsumer* fileConsumer = (FileDiagnosticConsumer*)consumer;
    FILE* output = fileConsumer->output ? fileConsumer->output : stderr;

    char* formatted = formatDiagnostic(diagnostic);
    if (formatted) {
        fprintf(output, "%s\n", formatted);
        free(formatted);
    }
}

static void fileDiagnosticConsumerDestroy(DiagnosticConsumer* consumer) {
    if (consumer) {
        FileDiagnosticConsumer* fileConsumer = (FileDiagnosticConsumer*)consumer;
        if (fileConsumer->shouldClose && fileConsumer->output) {
            fclose(fileConsumer->output);
        }
        free(consumer);
    }
}

DiagnosticConsumer* createFileDiagnosticConsumer(FILE* output) {
    FileDiagnosticConsumer* consumer = (FileDiagnosticConsumer*)malloc(sizeof(FileDiagnosticConsumer));
    if (!consumer) {
        return NULL;
    }

    consumer->base.handleDiagnostic = fileDiagnosticConsumerHandle;
    consumer->base.destroy = fileDiagnosticConsumerDestroy;
    consumer->base.privateData = NULL;
    consumer->output = output ? output : stderr;
    consumer->shouldClose = false;  // 不关闭用户提供的文件

    return (DiagnosticConsumer*)consumer;
}

/**
 * @brief 内存缓冲消费者
 */
typedef struct {
    DiagnosticConsumer base;
    char* buffer;
    size_t bufferSize;
    size_t offset;
} BufferDiagnosticConsumer;

static void bufferDiagnosticConsumerHandle(DiagnosticConsumer* consumer,
                                          const Diagnostic* diagnostic) {
    if (!consumer || !diagnostic) {
        return;
    }

    BufferDiagnosticConsumer* bufferConsumer = (BufferDiagnosticConsumer*)consumer;

    char* formatted = formatDiagnostic(diagnostic);
    if (formatted) {
        size_t len = strlen(formatted);
        if (bufferConsumer->offset + len + 1 < bufferConsumer->bufferSize) {
            strcpy(bufferConsumer->buffer + bufferConsumer->offset, formatted);
            bufferConsumer->offset += len;
            bufferConsumer->buffer[bufferConsumer->offset++] = '\n';
            bufferConsumer->buffer[bufferConsumer->offset] = '\0';
        }
        free(formatted);
    }
}

static void bufferDiagnosticConsumerDestroy(DiagnosticConsumer* consumer) {
    if (consumer) {
        free(consumer);
    }
}

DiagnosticConsumer* createBufferDiagnosticConsumer(char* buffer, size_t bufferSize) {
    BufferDiagnosticConsumer* consumer = (BufferDiagnosticConsumer*)malloc(sizeof(BufferDiagnosticConsumer));
    if (!consumer) {
        return NULL;
    }

    consumer->base.handleDiagnostic = bufferDiagnosticConsumerHandle;
    consumer->base.destroy = bufferDiagnosticConsumerDestroy;
    consumer->base.privateData = NULL;
    consumer->buffer = buffer;
    consumer->bufferSize = bufferSize;
    consumer->offset = 0;

    if (buffer && bufferSize > 0) {
        buffer[0] = '\0';
    }

    return (DiagnosticConsumer*)consumer;
}

void destroyDiagnosticConsumer(DiagnosticConsumer* consumer) {
    if (consumer && consumer->destroy) {
        consumer->destroy(consumer);
    }
}

// ==================== 诊断引擎实现 ====================

DiagnosticEngine* createDiagnosticEngine(DiagnosticConsumer* consumer) {
    DiagnosticEngine* engine = (DiagnosticEngine*)malloc(sizeof(DiagnosticEngine));
    if (!engine) {
        return NULL;
    }

    // 如果没有提供消费者，使用默认的stderr消费者
    if (!consumer) {
        consumer = createStderrDiagnosticConsumer();
    }

    engine->consumer = consumer;
    engine->errorCount = 0;
    engine->warningCount = 0;
    engine->suppressErrors = false;
    engine->suppressWarnings = false;
    engine->fatalErrorOccurred = false;
    engine->privateData = NULL;

    return engine;
}

void destroyDiagnosticEngine(DiagnosticEngine* engine) {
    if (engine) {
        if (engine->consumer) {
            destroyDiagnosticConsumer(engine->consumer);
        }
        if (engine->privateData) {
            free(engine->privateData);
        }
        free(engine);
    }
}

void diagnosticEngineReport(DiagnosticEngine* engine,
                           DiagnosticLevel level,
                           SourceLocation location,
                           const char* format,
                           ...) {
    if (!engine || !format) {
        return;
    }

    // 检查是否抑制
    if (level == DIAGNOSTIC_LEVEL_WARNING && engine->suppressWarnings) {
        return;
    }
    if ((level == DIAGNOSTIC_LEVEL_ERROR || level == DIAGNOSTIC_LEVEL_FATAL) &&
        engine->suppressErrors) {
        return;
    }

    // 更新计数
    if (level == DIAGNOSTIC_LEVEL_ERROR || level == DIAGNOSTIC_LEVEL_FATAL) {
        engine->errorCount++;
        if (level == DIAGNOSTIC_LEVEL_FATAL) {
            engine->fatalErrorOccurred = true;
        }
    } else if (level == DIAGNOSTIC_LEVEL_WARNING) {
        engine->warningCount++;
    }

    // 格式化消息
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

    // 创建诊断消息
    Diagnostic diagnostic = {
        .level = level,
        .location = location,
        .message = message ? message : (char*)format,
        .category = NULL,
        .isError = (level == DIAGNOSTIC_LEVEL_ERROR || level == DIAGNOSTIC_LEVEL_FATAL)
    };

    // 发送给消费者
    if (engine->consumer && engine->consumer->handleDiagnostic) {
        engine->consumer->handleDiagnostic(engine->consumer, &diagnostic);
    }

    if (message) {
        free(message);
    }
}

size_t diagnosticEngineGetErrorCount(const DiagnosticEngine* engine) {
    return engine ? engine->errorCount : 0;
}

size_t diagnosticEngineGetWarningCount(const DiagnosticEngine* engine) {
    return engine ? engine->warningCount : 0;
}

bool diagnosticEngineHasErrors(const DiagnosticEngine* engine) {
    return engine ? engine->errorCount > 0 : false;
}

void diagnosticEngineResetCounters(DiagnosticEngine* engine) {
    if (engine) {
        engine->errorCount = 0;
        engine->warningCount = 0;
        engine->fatalErrorOccurred = false;
    }
}

void diagnosticEngineSetSuppressErrors(DiagnosticEngine* engine, bool suppress) {
    if (engine) {
        engine->suppressErrors = suppress;
    }
}

void diagnosticEngineSetSuppressWarnings(DiagnosticEngine* engine, bool suppress) {
    if (engine) {
        engine->suppressWarnings = suppress;
    }
}
