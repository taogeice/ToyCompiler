# 语法分析器模块
# 提供：解析器接口、语法规则、运算符优先级、错误恢复

add_library(toycompiler_parser STATIC
    parser.h
    parser.c
    grammar.h
    grammar.c
    precedence.c
    error_recovery.c
)

target_include_directories(toycompiler_parser
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/frontend/parser
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_parser
    PUBLIC
        toycompiler_lexer
        toycompiler_diagnostics
)

# 设置别名
add_library(frontend::parser ALIAS toycompiler_parser)
