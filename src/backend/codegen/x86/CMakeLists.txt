# x86架构代码生成模块
# 提供：x86后端、汇编器、指令定义

add_library(toycompiler_x86 STATIC
    x86_backend.c
    x86_assembler.c
    x86_instructions.c
)

target_include_directories(toycompiler_x86
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/backend/codegen/x86
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_x86
    PUBLIC
        toycompiler_codegen_formats
)

# 设置别名
add_library(backend::x86 ALIAS toycompiler_x86)
