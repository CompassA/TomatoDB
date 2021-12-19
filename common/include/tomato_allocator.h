/*
 * @Author: Tomato
 * @Date: 2021-12-18 13:08:13
 * @LastEditTime: 2021-12-19 13:28:47
 */
#ifndef TOMATODB_COMMON_INCLUDE_TOMATODB_ALLOCATOR_H
#define TOMATODB_COMMON_INCLUDE_TOMATODB_ALLOCATOR_H
#include <vector>
#include <atomic>

namespace tomato {

/**
 * @brief 内存分配器
 * 
 */
class Allocator {
public:
    Allocator();

    /**
     * @brief 回收所有分配过的内存，分配器分配过的内存只有在分配器析构时才会被回收
     * 
     */
    ~Allocator();

    /**
     * @brief 分配内存
     * 
     * @param bytes 要分配的内存
     * @return char* 内存块首地址
     */
    char* allocate(size_t bytes);

    /**
     * @brief 分配内存，内存首地址是对齐的
     * 
     * @param bytes 要分配的内存
     * @return char* 内存块首地址
     */
    char* allocateAligned(size_t bytes);

    /**
     * @brief 获取内存分配池已经分配的内存字节数
     * 
     * @return 字节
     */
    size_t getAllocatedSize() const {
        return allocated_size_.load(std::memory_order_relaxed);
    }

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;
private:
    char* doAllocate(size_t bytes);
private:
    // 当前分配器分配过的所有内存块指针
    std::vector<char*> pool_;

    // 当前可分配内存的首地址
    char* pool_begin_;

    // 当前可分配内存的剩余容量
    size_t current_remaining_;

    // 已经分配了多少内存
    std::atomic<size_t> allocated_size_;
};

}


#endif