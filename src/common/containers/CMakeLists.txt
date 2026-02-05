# 容器数据结构模块
# 提供：动态数组、双端队列、二叉搜索树、图结构

add_library(toycompiler_containers STATIC
    vector.h
    vector.c
    deque.h
    deque.c
    bst.h
    bst.c
    graph.h
    graph.c
)

target_include_directories(toycompiler_containers
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/common/containers
)

# 设置别名
add_library(common::containers ALIAS toycompiler_containers)
