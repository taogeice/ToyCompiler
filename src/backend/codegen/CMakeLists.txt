# 代码生成器模块
# 提供：代码生成接口、目标机器描述、指令选择

add_library(toycompiler_backend_codegen STATIC
    codegen.h
    codegen.c
    target_machine.h
    target_machine.c
    instruction_selector.c
)

target_include_directories(toycompiler_backend_codegen
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/backend/codegen
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_backend_codegen
    PUBLIC
        toycompiler_ir
        toycompiler_type
)

# x86架构支持
add_subdirectory(x86)

# 设置别名
add_library(backend::codegen ALIAS toycompiler_backend_codegen)
