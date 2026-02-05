# 输入输出模块
# 提供：缓冲区、文件读取器、输出写入器

add_library(toycompiler_io STATIC
    buffer.h
    buffer.c
    file_reader.h
    file_reader.c
    output_writer.h
    output_writer.c
)

target_include_directories(toycompiler_io
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/common/io
)

# 设置别名
add_library(common::io ALIAS toycompiler_io)
