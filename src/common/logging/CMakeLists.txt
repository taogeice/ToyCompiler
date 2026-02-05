# 日志系统模块
# 提供：日志级别定义、日志记录器、日志格式化

add_library(toycompiler_logging STATIC
    log_level.h
    logger.h
    logger.cpp
    log_formatter.cpp
)

target_include_directories(toycompiler_logging
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/common/logging
)

# 设置别名
add_library(common::logging ALIAS toycompiler_logging)
