# 抽象语法树模块
# 提供：AST核心定义、AST节点、访问者模式、AST构建器

add_library(toycompiler_ast STATIC
    ast.h
    ast.c
    ast_nodes.h
    ast_nodes.c
    ast_visitor.h
    ast_visitor.c
    ast_builder.h
    ast_builder.c
    ast_dumper.h
    ast_dumper.c
    ast_utils.h
    ast_utils.c
)

target_include_directories(toycompiler_ast
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/frontend/ast
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_ast
    PUBLIC
        toycompiler_lexer
        toycompiler_parser
        toycompiler_diagnostics
)

# 设置别名
add_library(frontend::ast ALIAS toycompiler_ast)
