# 中间表示(IR)模块
# 提供：IR核心定义、IR指令、基本块、函数、模块表示

add_library(toycompiler_ir STATIC
    ir.h
    ir_instruction.h
    ir_instruction.c
    ir_types.c
    basic_block.c
    function.c
    module.c
    ir_builder.c
)

target_include_directories(toycompiler_ir
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/midend/ir
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_ir
    PUBLIC
        toycompiler_type
        toycompiler_symbol
        toycompiler_containers
)

# 设置别名
add_library(midend::ir ALIAS toycompiler_ir)
