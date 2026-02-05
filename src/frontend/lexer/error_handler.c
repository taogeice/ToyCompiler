#include "error_handler.h"
#include "../../common/containers/vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================== 内部辅助函数 ====================

static void destroyErrorInfo(void* error) {
    if (!error) {
        return;
    }

    ErrorInfo* errorInfo = (ErrorInfo*)error;

    if (errorInfo->message) {
        free(errorInfo->message);
    }

    if (errorInfo->suggestion) {
        free(errorInfo->suggestion);
    }

    if (errorInfo->location.filename) {
        free(errorInfo->location.filename);
    }
}

// ==================== 构造函数和析构函数 ====================

ErrorHandler* createErrorHandler(void) {
    ErrorHandler* handler = (ErrorHandler*)malloc(sizeof(ErrorHandler));
    if (!handler) {
        return NULL;
    }

    handler->errors = vectorCreate(sizeof(ErrorInfo), 16);
    if (!handler->errors) {
        free(handler);
        return NULL;
    }

    handler->hasError = false;
    handler->hasWarning = false;
    handler->errorCount = 0;
    handler->warningCount = 0;
    handler->verbose = false;
    handler->stopOnError = false;

    return handler;
}

void destroyErrorHandler(ErrorHandler* handler) {
    if (!handler) {
        return;
    }

    if (handler->errors) {
        vectorDestroy(handler->errors, destroyErrorInfo);
    }

    free(handler);
}

// ==================== 错误报告 ====================

bool reportError(ErrorHandler* handler, ErrorLevel level, ErrorType type,
                SourceLocation location, const char* message, const char* suggestion) {
    if (!handler || !message) {
        return false;
    }

    ErrorInfo error;
    error.level = level;
    error.type = type;
    error.location = location;
    error.message = strdup(message);
    error.suggestion = suggestion ? strdup(suggestion) : NULL;
    error.errorCode = 0;

    if (location.filename) {
        error.location.filename = strdup(location.filename);
    }

    if (!vectorPushBack(handler->errors, &error)) {
        if (error.message) free(error.message);
        if (error.suggestion) free(error.suggestion);
        if (error.location.filename) free(error.location.filename);
        return false;
    }

    if (level == ERROR_LEVEL_ERROR || level == ERROR_LEVEL_FATAL) {
        handler->errorCount++;
        handler->hasError = true;
    } else if (level == ERROR_LEVEL_WARNING) {
        handler->warningCount++;
        handler->hasWarning = true;
    }

    return true;
}

bool reportLexicalError(ErrorHandler* handler, SourceLocation location, const char* message) {
    return reportError(handler, ERROR_LEVEL_ERROR, ERROR_TYPE_LEXICAL, location, message, NULL);
}

bool reportWarning(ErrorHandler* handler, SourceLocation location, const char* message) {
    return reportError(handler, ERROR_LEVEL_WARNING, ERROR_TYPE_LEXICAL, location, message, NULL);
}

bool reportFatalError(ErrorHandler* handler, SourceLocation location, const char* message) {
    return reportError(handler, ERROR_LEVEL_FATAL, ERROR_TYPE_LEXICAL, location, message, NULL);
}

// ==================== 错误查询 ====================

int getErrorCount(const ErrorHandler* handler) {
    return handler ? handler->errorCount : 0;
}

int getWarningCount(const ErrorHandler* handler) {
    return handler ? handler->warningCount : 0;
}

bool hasErrors(const ErrorHandler* handler) {
    return handler ? handler->hasError : false;
}

bool hasWarnings(const ErrorHandler* handler) {
    return handler ? handler->hasWarning : false;
}

// ==================== 错误输出 ====================

void printErrors(const ErrorHandler* handler) {
    if (!handler || !handler->errors) {
        return;
    }

    size_t errorCount = vectorSize(handler->errors);

    for (size_t i = 0; i < errorCount; i++) {
        ErrorInfo* error = (ErrorInfo*)vectorGet(handler->errors, i);
        if (!error) {
            continue;
        }

        char* formatted = formatErrorMessage(error);
        if (formatted) {
            fprintf(stderr, "%s\n", formatted);
            free(formatted);
        }
    }
}

void printErrorSummary(const ErrorHandler* handler) {
    if (!handler) {
        return;
    }

    if (handler->errorCount == 0 && handler->warningCount == 0) {
        printf("No errors or warnings.\n");
        return;
    }

    printf("\n=== Error Summary ===\n");
    printf("Errors: %d\n", handler->errorCount);
    printf("Warnings: %d\n", handler->warningCount);
    printf("====================\n");
}

void clearErrors(ErrorHandler* handler) {
    if (!handler) {
        return;
    }

    vectorClear(handler->errors, destroyErrorInfo);

    handler->hasError = false;
    handler->hasWarning = false;
    handler->errorCount = 0;
    handler->warningCount = 0;
}

// ==================== 错误处理辅助函数 ====================

char* formatErrorMessage(const ErrorInfo* error) {
    if (!error || !error->message) {
        return strdup("Invalid error");
    }

    char* locationStr = sourceLocationToString(&error->location);
    const char* levelStr = errorLevelToString(error->level);
    const char* typeStr = errorTypeToString(error->type);

    char* result = NULL;

    if (error->suggestion) {
        asprintf(&result, "%s:%s: %s: %s\n  Suggestion: %s",
                 locationStr, levelStr, typeStr, error->message, error->suggestion);
    } else {
        asprintf(&result, "%s:%s: %s: %s",
                 locationStr, levelStr, typeStr, error->message);
    }

    if (locationStr) {
        free(locationStr);
    }

    return result ? result : strdup("Failed to format error");
}

const char* errorLevelToString(ErrorLevel level) {
    switch (level) {
        case ERROR_LEVEL_WARNING:
            return "warning";
        case ERROR_LEVEL_ERROR:
            return "error";
        case ERROR_LEVEL_FATAL:
            return "fatal error";
        default:
            return "unknown";
    }
}

const char* errorTypeToString(ErrorType type) {
    switch (type) {
        case ERROR_TYPE_LEXICAL:
            return "lexical";
        case ERROR_TYPE_SYNTAX:
            return "syntax";
        case ERROR_TYPE_SEMANTIC:
            return "semantic";
        case ERROR_TYPE_IO:
            return "I/O";
        case ERROR_TYPE_MEMORY:
            return "memory";
        default:
            return "unknown";
    }
}
