cmake_minimum_required(VERSION 3.15)
project(ToyCompiler VERSION 0.1.0 LANGUAGES C CXX)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(MSVC)
    add_compile_options(/W4)
endif()

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 子目录
add_subdirectory(src/common)
add_subdirectory(src/type)
add_subdirectory(src/symbol)
add_subdirectory(src/frontend)
add_subdirectory(src/midend)
add_subdirectory(src/backend)
add_subdirectory(src/codegen)
add_subdirectory(src/driver)

# 主可执行文件
add_executable(toycompiler
    src/driver/main.cpp
)

target_link_libraries(toycompiler
    PRIVATE
        toycompiler_driver
        toycompiler_codegen_formats
        toycompiler_backend
        toycompiler_midend
        toycompiler_frontend
        toycompiler_symbol
        toycompiler_type
        toycompiler_common
)

# 安装规则
install(TARGETS toycompiler DESTINATION bin)

# 测试选项
option(BUILD_TESTING "构建测试" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# 文档选项
option(BUILD_DOCS "构建文档" OFF)
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

# 打印配置信息
message(STATUS "")
message(STATUS "==================== 配置摘要 ====================")
message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "版本号: ${PROJECT_VERSION}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "C编译器: ${CMAKE_C_COMPILER}")
message(STATUS "C++编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "C标准: ${CMAKE_C_STANDARD}")
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "构建测试: ${BUILD_TESTING}")
message(STATUS "构建文档: ${BUILD_DOCS}")
message(STATUS "==================================================")
message(STATUS "")
