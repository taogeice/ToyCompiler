# 后端模块
# 包含：代码生成器、寄存器分配

# 添加子目录
add_subdirectory(codegen)
add_subdirectory(registeralloc)

# 创建接口库，用于统一导出
add_library(toycompiler_backend INTERFACE)

target_link_libraries(toycompiler_backend
    INTERFACE
        toycompiler_codegen_formats
        toycompiler_x86
        toycompiler_regalloc
)

# 设置别名
add_library(backend::backend ALIAS toycompiler_backend)
