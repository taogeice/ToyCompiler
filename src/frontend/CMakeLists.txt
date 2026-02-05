# 前端模块
# 包含：词法分析、语法分析、AST、语义分析

# 添加子目录
add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(ast)
add_subdirectory(semanticanalyzer)

# 创建接口库，用于统一导出
add_library(toycompiler_frontend INTERFACE)

target_link_libraries(toycompiler_frontend
    INTERFACE
        toycompiler_lexer
        toycompiler_parser
        toycompiler_ast
        toycompiler_semantic
)

# 设置别名
add_library(frontend::frontend ALIAS toycompiler_frontend)
