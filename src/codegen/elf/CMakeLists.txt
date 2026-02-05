# ELF格式模块 (Linux)
# 提供：ELF构建器、节管理器、字符串表、符号表、重定位

add_library(toycompiler_elf STATIC
    elf_builder.cpp
    section_manager.cpp
    string_table.cpp
    symbol_table.cpp
    relocation.cpp
)

target_include_directories(toycompiler_elf
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/codegen/elf
        ${CMAKE_SOURCE_DIR}/src
)

# 设置别名
add_library(codegen::elf ALIAS toycompiler_elf)
