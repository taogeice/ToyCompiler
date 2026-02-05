# 配置管理模块
# 提供：配置接口、命令行处理、目标配置

add_library(toycompiler_config STATIC
    config.h
    config.c
    command_line.c
    target_config.c
)

target_include_directories(toycompiler_config
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/common/config
)

# 设置别名
add_library(common::config ALIAS toycompiler_config)
