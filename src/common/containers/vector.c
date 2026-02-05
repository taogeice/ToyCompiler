#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>  // 用于SIZE_MAX

// 默认初始容量
#define DEFAULT_CAPACITY 4

// 扩容因子
#define GROWTH_FACTOR 2

// ==================== 内部辅助函数 ====================

/**
 * @brief 计算新的扩容容量
 */
static inline size_t calculateNewCapacity(size_t currentCapacity) {
    size_t newCapacity = currentCapacity * GROWTH_FACTOR;
    // 避免容量为0
    return newCapacity == 0 ? DEFAULT_CAPACITY : newCapacity;
}

/**
 * @brief 检查并确保Vector有足够的容量
 * @return 成功返回true，失败返回false
 */
static bool vectorEnsureCapacity(Vector* vector, size_t requiredCapacity) {
    if (vector->capacity >= requiredCapacity) {
        return true;
    }

    size_t newCapacity = calculateNewCapacity(vector->capacity);
    
    // 确保新容量足够
    while (newCapacity < requiredCapacity) {
        newCapacity = calculateNewCapacity(newCapacity);
    }

    // 重新分配内存
    void* newData = realloc(vector->data, newCapacity * vector->elementSize);
    if (!newData) {
        return false;
    }

    vector->data = newData;
    vector->capacity = newCapacity;

    return true;
}

// ==================== 构造函数和析构函数 ====================

Vector* vectorCreate(size_t elementSize, size_t initialCapacity) {
    if (elementSize == 0) {
        return NULL;
    }

    Vector* vector = (Vector*)malloc(sizeof(Vector));
    if (!vector) {
        return NULL;
    }

    // 使用默认容量或指定容量
    size_t capacity = (initialCapacity == 0) ? DEFAULT_CAPACITY : initialCapacity;

    // 分配元素数组
    vector->data = malloc(capacity * elementSize);
    if (!vector->data) {
        free(vector);
        return NULL;
    }

    vector->elementSize = elementSize;
    vector->size = 0;
    vector->capacity = capacity;

    return vector;
}

void vectorDestroy(Vector* vector, void (*elementDestructor)(void*)) {
    if (!vector) {
        return;
    }

    // 如果有元素析构函数，先调用它
    if (elementDestructor && vector->size > 0) {
        for (size_t i = 0; i < vector->size; i++) {
            void* element = (char*)vector->data + i * vector->elementSize;
            elementDestructor(element);
        }
    }

    // 释放元素数组
    if (vector->data) {
        free(vector->data);
    }

    // 释放Vector结构体
    free(vector);
}

// ==================== 元素访问 ====================

void* vectorGet(const Vector* vector, size_t index) {
    if (!vector || index >= vector->size) {
        return NULL;
    }

    return (char*)vector->data + index * vector->elementSize;
}

void* vectorFront(const Vector* vector) {
    return vectorGet(vector, 0);
}

void* vectorBack(const Vector* vector) {
    if (!vector || vector->size == 0) {
        return NULL;
    }
    return vectorGet(vector, vector->size - 1);
}

void* vectorData(const Vector* vector) {
    return vector ? vector->data : NULL;
}

// ==================== 容量操作 ====================

size_t vectorSize(const Vector* vector) {
    return vector ? vector->size : 0;
}

size_t vectorCapacity(const Vector* vector) {
    return vector ? vector->capacity : 0;
}

bool vectorIsEmpty(const Vector* vector) {
    return vector ? vector->size == 0 : true;
}

bool vectorReserve(Vector* vector, size_t newCapacity) {
    if (!vector || newCapacity <= vector->capacity) {
        return false;
    }

    // 重新分配内存
    void* newData = realloc(vector->data, newCapacity * vector->elementSize);
    if (!newData) {
        return false;
    }

    vector->data = newData;
    vector->capacity = newCapacity;

    return true;
}

bool vectorResize(Vector* vector, size_t newSize, const void* value) {
    if (!vector) {
        return false;
    }

    // 如果新大小小于当前大小，截断
    if (newSize < vector->size) {
        vector->size = newSize;
        return true;
    }

    // 如果新大小大于当前大小，扩容并填充
    if (newSize > vector->size) {
        if (!vectorEnsureCapacity(vector, newSize)) {
            return false;
        }

        // 填充新元素
        if (value) {
            for (size_t i = vector->size; i < newSize; i++) {
                void* element = (char*)vector->data + i * vector->elementSize;
                memcpy(element, value, vector->elementSize);
            }
        }

        vector->size = newSize;
    }

    return true;
}

bool vectorShrinkToFit(Vector* vector) {
    if (!vector || vector->size == vector->capacity) {
        return false;
    }

    // 如果Vector为空，释放所有内存
    if (vector->size == 0) {
        free(vector->data);
        vector->data = NULL;
        vector->capacity = 0;
        return true;
    }

    // 重新分配到刚好容纳当前元素的大小
    void* newData = realloc(vector->data, vector->size * vector->elementSize);
    if (!newData) {
        return false;
    }

    vector->data = newData;
    vector->capacity = vector->size;

    return true;
}

// ==================== 修改操作 ====================

bool vectorPushBack(Vector* vector, const void* element) {
    if (!vector || !element) {
        return false;
    }

    // 确保有足够的容量
    if (!vectorEnsureCapacity(vector, vector->size + 1)) {
        return false;
    }

    // 复制元素到末尾
    void* dest = (char*)vector->data + vector->size * vector->elementSize;
    memcpy(dest, element, vector->elementSize);

    vector->size++;

    return true;
}

bool vectorPopBack(Vector* vector, void (*elementDestructor)(void*)) {
    if (!vector || vector->size == 0) {
        return false;
    }

    // 调用元素析构函数
    if (elementDestructor) {
        void* element = (char*)vector->data + (vector->size - 1) * vector->elementSize;
        elementDestructor(element);
    }

    vector->size--;

    return true;
}

bool vectorInsert(Vector* vector, size_t index, const void* element) {
    if (!vector || !element || index > vector->size) {
        return false;
    }

    // 确保有足够的容量
    if (!vectorEnsureCapacity(vector, vector->size + 1)) {
        return false;
    }

    // 移动元素为新元素腾出空间
    if (index < vector->size) {
        void* dest = (char*)vector->data + (index + 1) * vector->elementSize;
        void* src = (char*)vector->data + index * vector->elementSize;
        size_t moveSize = (vector->size - index) * vector->elementSize;
        memmove(dest, src, moveSize);
    }

    // 插入新元素
    void* dest = (char*)vector->data + index * vector->elementSize;
    memcpy(dest, element, vector->elementSize);

    vector->size++;

    return true;
}

bool vectorErase(Vector* vector, size_t index, void (*elementDestructor)(void*)) {
    if (!vector || index >= vector->size) {
        return false;
    }

    // 调用元素析构函数
    if (elementDestructor) {
        void* element = (char*)vector->data + index * vector->elementSize;
        elementDestructor(element);
    }

    // 移动元素填补空缺
    if (index < vector->size - 1) {
        void* dest = (char*)vector->data + index * vector->elementSize;
        void* src = (char*)vector->data + (index + 1) * vector->elementSize;
        size_t moveSize = (vector->size - index - 1) * vector->elementSize;
        memmove(dest, src, moveSize);
    }

    vector->size--;

    return true;
}

void vectorClear(Vector* vector, void (*elementDestructor)(void*)) {
    if (!vector) {
        return;
    }

    // 调用元素析构函数
    if (elementDestructor && vector->size > 0) {
        for (size_t i = 0; i < vector->size; i++) {
            void* element = (char*)vector->data + i * vector->elementSize;
            elementDestructor(element);
        }
    }

    vector->size = 0;
}

// ==================== 查找操作 ====================

size_t vectorFind(const Vector* vector, const void* element, 
                  int (*comparator)(const void*, const void*)) {
    if (!vector || !element || !comparator) {
        return SIZE_MAX;
    }

    for (size_t i = 0; i < vector->size; i++) {
        void* current = (char*)vector->data + i * vector->elementSize;
        if (comparator(current, element) == 0) {
            return i;
        }
    }

    return SIZE_MAX;
}

bool vectorContains(const Vector* vector, const void* element,
                    int (*comparator)(const void*, const void*)) {
    return vectorFind(vector, element, comparator) != SIZE_MAX;
}

// ==================== 迭代操作 ====================

void vectorForEach(Vector* vector, void (*action)(void*)) {
    if (!vector || !action) {
        return;
    }

    for (size_t i = 0; i < vector->size; i++) {
        void* element = (char*)vector->data + i * vector->elementSize;
        action(element);
    }
}

void vectorForEachConst(const Vector* vector, void (*action)(const void*)) {
    if (!vector || !action) {
        return;
    }

    for (size_t i = 0; i < vector->size; i++) {
        const void* element = (const char*)vector->data + i * vector->elementSize;
        action(element);
    }
}

// ==================== 实用函数 ====================

void vectorSwap(Vector* vector1, Vector* vector2) {
    if (!vector1 || !vector2) {
        return;
    }

    Vector temp = *vector1;
    *vector1 = *vector2;
    *vector2 = temp;
}

void vectorReverse(Vector* vector) {
    if (!vector || vector->size <= 1) {
        return;
    }

    // 临时缓冲区
    void* temp = malloc(vector->elementSize);
    if (!temp) {
        return;
    }

    for (size_t i = 0; i < vector->size / 2; i++) {
        void* left = (char*)vector->data + i * vector->elementSize;
        void* right = (char*)vector->data + (vector->size - 1 - i) * vector->elementSize;

        // 交换
        memcpy(temp, left, vector->elementSize);
        memcpy(left, right, vector->elementSize);
        memcpy(right, temp, vector->elementSize);
    }

    free(temp);
}

void vectorSort(Vector* vector, int (*comparator)(const void*, const void*)) {
    if (!vector || !comparator || vector->size <= 1) {
        return;
    }

    qsort(vector->data, vector->size, vector->elementSize, comparator);
}
