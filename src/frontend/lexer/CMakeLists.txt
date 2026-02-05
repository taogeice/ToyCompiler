# 词法分析器模块
# 提供：Token定义、词法分析器核心实现

add_library(toycompiler_lexer STATIC
    token.h
    token.c
    lexer.h
    lexer.c
    tokenizer.c
    error_handler.c
)

target_include_directories(toycompiler_lexer
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/frontend/lexer
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_lexer
    PUBLIC
        toycompiler_diagnostics
        toycompiler_utils
)

# 设置别名
add_library(frontend::lexer ALIAS toycompiler_lexer)
