# Mach-O格式模块 (macOS)
# 提供：Mach-O构建器、Mach-O节管理

add_library(toycompiler_mach_o STATIC
    mach_o_builder.cpp
    mach_o_sections.cpp
)

target_include_directories(toycompiler_mach_o
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/codegen/mach_o
        ${CMAKE_SOURCE_DIR}/src
)

# 设置别名
add_library(codegen::mach_o ALIAS toycompiler_mach_o)
