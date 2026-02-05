# 编译器驱动程序模块
# 提供：编译器驱动、命令行处理、编译单元、流水线管理

add_library(toycompiler_driver STATIC
    compiler_driver.h
    compiler_driver.cpp
    command_line.cpp
    compilation_unit.cpp
    pipeline_manager.cpp
    target_detection.cpp
)

target_include_directories(toycompiler_driver
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/driver
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(toycompiler_driver
    PUBLIC
        toycompiler_frontend
        toycompiler_midend
        toycompiler_backend
        toycompiler_codegen_formats
        toycompiler_common
)

# 设置别名
add_library(driver::driver ALIAS toycompiler_driver)
