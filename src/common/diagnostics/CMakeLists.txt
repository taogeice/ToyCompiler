# 诊断系统模块
# 提供：源位置跟踪、诊断引擎、错误报告器

add_library(toycompiler_diagnostics STATIC
    source_location.c
    source_location.h
    diagnostic_engine.h
    diagnostic_engine.c
    diagnostic_consumer.h
    error_reporter.h
    error_reporter.c
    formatting.c
)

target_include_directories(toycompiler_diagnostics
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/common/diagnostics
)

# 设置别名
add_library(common::diagnostics ALIAS toycompiler_diagnostics)
