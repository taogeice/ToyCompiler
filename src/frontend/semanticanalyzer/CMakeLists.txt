# 语义分析器模块
# 提供：语义分析接口、类型检查、作用域分析、常量求值

add_library(toycompiler_semantic STATIC
    semanticanalyzer.h
    semanticanalyzer.c
    typechecker.c
    scope_analyzer.c
    constant_evaluator.c
    diagnostics.c
)

target_include_directories(toycompiler_semantic
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/frontend/semanticanalyzer
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_semantic
    PUBLIC
        toycompiler_ast
        toycompiler_parser
        toycompiler_type
        toycompiler_symbol
        toycompiler_diagnostics
)

# 设置别名
add_library(frontend::semantic ALIAS toycompiler_semantic)
