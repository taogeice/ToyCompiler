# 工具函数模块
# 提供：内存池、哈希表、字符串工具、文件工具

add_library(toycompiler_utils STATIC
    memory_pool.h
    memory_pool.c
    hash_table.h
    hash_table.c
    string_utils.h
    string_utils.c
    file_utils.c
    container_utils.h
)

target_include_directories(toycompiler_utils
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/common/utils
)

# 设置别名
add_library(common::utils ALIAS toycompiler_utils)
