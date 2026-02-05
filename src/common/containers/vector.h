#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Vector动态数组结构体
 * 
 * 泛型动态数组，可存储任意类型的数据
 */
typedef struct {
    void* data;          // 指向元素数组的指针
    size_t elementSize;  // 每个元素的大小（字节）
    size_t size;         // 当前元素数量
    size_t capacity;     // 当前容量（可容纳的元素数）
} Vector;

// ==================== 构造函数和析构函数 ====================

/**
 * @brief 创建Vector
 * @param elementSize 每个元素的大小（字节）
 * @param initialCapacity 初始容量（0表示使用默认值4）
 * @return 新创建的Vector，失败返回NULL
 */
Vector* vectorCreate(size_t elementSize, size_t initialCapacity);

/**
 * @brief 销毁Vector
 * @param vector 要销毁的Vector
 * @param elementDestructor 元素析构函数（可为NULL，用于释放元素内部的动态内存）
 */
void vectorDestroy(Vector* vector, void (*elementDestructor)(void*));

// ==================== 元素访问 ====================

/**
 * @brief 获取指定位置的元素
 * @param vector Vector
 * @param index 元素索引
 * @return 指向元素的指针，越界返回NULL
 */
void* vectorGet(const Vector* vector, size_t index);

/**
 * @brief 获取第一个元素
 * @param vector Vector
 * @return 指向第一个元素的指针，空Vector返回NULL
 */
void* vectorFront(const Vector* vector);

/**
 * @brief 获取最后一个元素
 * @param vector Vector
 * @return 指向最后一个元素的指针，空Vector返回NULL
 */
void* vectorBack(const Vector* vector);

/**
 * @brief 获取Vector的底层数据指针
 * @param vector Vector
 * @return 指向底层数组的指针
 */
void* vectorData(const Vector* vector);

// ==================== 容量操作 ====================

/**
 * @brief 获取当前元素数量
 * @param vector Vector
 * @return 元素数量
 */
size_t vectorSize(const Vector* vector);

/**
 * @brief 获取当前容量
 * @param vector Vector
 * @return 容量
 */
size_t vectorCapacity(const Vector* vector);

/**
 * @brief 检查Vector是否为空
 * @param vector Vector
 * @return 空返回true，否则返回false
 */
bool vectorIsEmpty(const Vector* vector);

/**
 * @brief 预留容量
 * @param vector Vector
 * @param newCapacity 新容量
 * @return 成功返回true，失败返回false
 */
bool vectorReserve(Vector* vector, size_t newCapacity);

/**
 * @brief 调整Vector大小
 * @param vector Vector
 * @param newSize 新大小
 * @param value 用于填充新元素的值（可为NULL，此时新元素未初始化）
 * @return 成功返回true，失败返回false
 */
bool vectorResize(Vector* vector, size_t newSize, const void* value);

/**
 * @brief 请求移除未使用的容量
 * @param vector Vector
 * @return 成功返回true，失败返回false
 */
bool vectorShrinkToFit(Vector* vector);

// ==================== 修改操作 ====================

/**
 * @brief 在末尾添加元素
 * @param vector Vector
 * @param element 指向要添加的元素的指针
 * @return 成功返回true，失败返回false
 */
bool vectorPushBack(Vector* vector, const void* element);

/**
 * @brief 移除末尾元素
 * @param vector Vector
 * @param elementDestructor 元素析构函数（可为NULL）
 * @return 成功返回true，失败返回false
 */
bool vectorPopBack(Vector* vector, void (*elementDestructor)(void*));

/**
 * @brief 在指定位置插入元素
 * @param vector Vector
 * @param index 插入位置
 * @param element 指向要插入的元素的指针
 * @return 成功返回true，失败返回false
 */
bool vectorInsert(Vector* vector, size_t index, const void* element);

/**
 * @brief 移除指定位置的元素
 * @param vector Vector
 * @param index 要移除的元素索引
 * @param elementDestructor 元素析构函数（可为NULL）
 * @return 成功返回true，失败返回false
 */
bool vectorErase(Vector* vector, size_t index, void (*elementDestructor)(void*));

/**
 * @brief 清空Vector（不释放内存）
 * @param vector Vector
 * @param elementDestructor 元素析构函数（可为NULL）
 */
void vectorClear(Vector* vector, void (*elementDestructor)(void*));

// ==================== 查找操作 ====================

/**
 * @brief 查找元素
 * @param vector Vector
 * @param element 要查找的元素
 * @param comparator 比较函数（返回0表示相等）
 * @return 找到返回索引，未找到返回SIZE_MAX
 */
size_t vectorFind(const Vector* vector, const void* element, 
                  int (*comparator)(const void*, const void*));

/**
 * @brief 检查是否包含元素
 * @param vector Vector
 * @param element 要查找的元素
 * @param comparator 比较函数
 * @return 包含返回true，否则返回false
 */
bool vectorContains(const Vector* vector, const void* element,
                    int (*comparator)(const void*, const void*));

// ==================== 迭代操作 ====================

/**
 * @brief 对每个元素执行操作
 * @param vector Vector
 * @param action 操作函数
 */
void vectorForEach(Vector* vector, void (*action)(void*));

/**
 * @brief 对每个元素执行操作（const版本）
 * @param vector Vector
 * @param action 操作函数
 */
void vectorForEachConst(const Vector* vector, void (*action)(const void*));

// ==================== 实用函数 ====================

/**
 * @brief 交换两个Vector的内容
 * @param vector1 第一个Vector
 * @param vector2 第二个Vector
 */
void vectorSwap(Vector* vector1, Vector* vector2);

/**
 * @brief 反转Vector中的元素顺序
 * @param vector Vector
 */
void vectorReverse(Vector* vector);

/**
 * @brief 对Vector进行排序
 * @param vector Vector
 * @param comparator 比较函数
 */
void vectorSort(Vector* vector, int (*comparator)(const void*, const void*));

#endif // VECTOR_H
