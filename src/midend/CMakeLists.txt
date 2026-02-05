# 中端模块
# 包含：中间表示(IR)、优化器、代码分析

# 添加子目录
add_subdirectory(ir)
add_subdirectory(optimizer)

# 创建接口库，用于统一导出
add_library(toycompiler_midend INTERFACE)

target_link_libraries(toycompiler_midend
    INTERFACE
        toycompiler_ir
        toycompiler_optimizer
        toycompiler_opt_passes
)

# 设置别名
add_library(midend::midend ALIAS toycompiler_midend)
