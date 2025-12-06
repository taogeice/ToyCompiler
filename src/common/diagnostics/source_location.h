#ifndef SOURCE_LOCATION_H
#define SOURCE_LOCATION_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//源位置结构定义
typedef struct {
    char* filename; //源文件名称
    int line;   //行号（从1开始）
    int column; //列号（从1开始）
    int offset; //文件中的字符偏移量



} SourceLocation;


// 构造函数
SourceLocation createSourceLocation(const char* filename, int line, int column, int offset);
// 辅助函数
char* sourceLocationToString(const SourceLocation* location);

// 析构函数
void destroySourceLocation(SourceLocation* location);


#endif