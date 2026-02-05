# 优化Pass模块
# 提供：常量折叠、死代码消除、内联、循环优化等

add_library(toycompiler_opt_passes STATIC
    constant_folding.c
    dead_code_elimination.c
    common_subexpression.c
    inlining.c
    loop_optimization.c
    ssa_construction.c
)

target_include_directories(toycompiler_opt_passes
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/midend/optimizer
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_opt_passes
    PUBLIC
        toycompiler_ir
        toycompiler_optimizer
)

# 设置别名
add_library(midend::opt_passes ALIAS toycompiler_opt_passes)
