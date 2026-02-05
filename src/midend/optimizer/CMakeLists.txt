# 优化器模块
# 提供：优化器接口、Pass管理器、控制流分析、数据流分析

add_library(toycompiler_optimizer STATIC
    optimizer.h
    optimization_pass.h
    optimizer.c
    pass_manager.c
    control_flow.c
    dataflow.c
    liveness.c
)

target_include_directories(toycompiler_optimizer
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/midend/optimizer
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_optimizer
    PUBLIC
        toycompiler_ir
        toycompiler_type
)

# 优化Pass子目录
add_subdirectory(passes)

# 设置别名
add_library(midend::optimizer ALIAS toycompiler_optimizer)
