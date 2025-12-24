#include "source_location.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 创建源位置信息
SourceLocation createSourceLocation(const char* filename, int line, int column, int offset) {
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
    if (location->filename) {
        asprintf(&result, "%s:%d:%d", location->filename, location->line, location->column);
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
