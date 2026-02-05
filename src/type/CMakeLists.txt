# 类型系统模块
# 提供：类型定义、内置类型、复合类型、C11原子类型等

add_library(toycompiler_type STATIC
    types.h
    types.c
    builtin_types.c
    array_type.c
    pointer_type.c
    struct_type.c
    union_type.c
    function_type.c
    atomic_type.c
    aligned_type.c
    type_builder.c
    type_comparison.c
    type_layout.c
)

target_include_directories(toycompiler_type
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/type
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_type
    PUBLIC
        toycompiler_diagnostics
        toycompiler_utils
)

# 设置别名
add_library(type::type ALIAS toycompiler_type)
