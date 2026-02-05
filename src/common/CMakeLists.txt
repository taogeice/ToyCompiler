# 公共基础组件模块
# 包含：诊断系统、容器、工具、IO、配置、日志

# 添加子目录
add_subdirectory(diagnostics)
add_subdirectory(containers)
add_subdirectory(utils)
add_subdirectory(io)
add_subdirectory(config)
add_subdirectory(logging)

# 创建接口库，用于统一导出
add_library(toycompiler_common INTERFACE)

target_link_libraries(toycompiler_common
    INTERFACE
        toycompiler_diagnostics
        toycompiler_containers
        toycompiler_utils
        toycompiler_io
        toycompiler_config
        toycompiler_logging
)

# 设置别名
add_library(common::common ALIAS toycompiler_common)
