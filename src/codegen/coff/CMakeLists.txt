# COFF格式模块 (Windows)
# 提供：COFF构建器、COFF节管理

add_library(toycompiler_coff STATIC
    coff_builder.cpp
    coff_sections.cpp
)

target_include_directories(toycompiler_coff
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/codegen/coff
        ${CMAKE_SOURCE_DIR}/src
)

# 设置别名
add_library(codegen::coff ALIAS toycompiler_coff)
