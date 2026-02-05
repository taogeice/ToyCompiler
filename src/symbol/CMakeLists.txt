# 符号表管理模块
# 提供：符号定义、作用域管理、符号表、名称查找

add_library(toycompiler_symbol STATIC
    symbol.h
    symbol.c
    scope.h
    scope.c
    symbol_table.h
    symbol_table.c
    name_lookup.c
    symbol_visitor.c
)

target_include_directories(toycompiler_symbol
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/symbol
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_symbol
    PUBLIC
        toycompiler_diagnostics
        toycompiler_containers
)

# 设置别名
add_library(symbol::symbol ALIAS toycompiler_symbol)
