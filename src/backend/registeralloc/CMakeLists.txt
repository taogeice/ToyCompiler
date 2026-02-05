# 寄存器分配模块
# 提供：寄存器分配接口、线性扫描、图着色等算法

add_library(toycompiler_regalloc STATIC
    register_alloc.h
    register_alloc.c
    linear_scan.c
    graph_coloring.c
    interference_graph.c
    liveness_analysis.c
    spill_strategy.c
)

target_include_directories(toycompiler_regalloc
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/backend/registeralloc
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_regalloc
    PUBLIC
        toycompiler_ir
        toycompiler_containers
)

# 设置别名
add_library(backend::regalloc ALIAS toycompiler_regalloc)
