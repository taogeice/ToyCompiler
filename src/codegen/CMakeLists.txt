# 目标代码格式支持模块
# 包含：ELF、COFF、Mach-O格式支持

# ELF格式 (Linux)
add_subdirectory(elf)

# COFF格式 (Windows)
add_subdirectory(coff)

# Mach-O格式 (macOS)
add_subdirectory(mach_o)

# 创建接口库，用于统一导出
add_library(toycompiler_codegen_formats INTERFACE)

target_link_libraries(toycompiler_codegen_formats
    INTERFACE
        toycompiler_elf
        toycompiler_coff
        toycompiler_mach_o
)

# 设置别名
add_library(codegen::formats ALIAS toycompiler_codegen_formats)
