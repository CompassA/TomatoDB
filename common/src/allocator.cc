/*
 * @Author: Tomato
 * @Date: 2021-12-18 13:19:56
 * @LastEditTime: 2023-02-01 23:01:45
 */
#include <tomato_common/allocator.h>
#include <cassert>

namespace tomato {

Allocator::~Allocator() {
    for (size_t i = 0; i < pool_.size(); ++i) {
        delete[] pool_[i];
    }
}

char* Allocator::allocate(size_t bytes) {
    assert(bytes > 0);

    // 仍有可用内存
    if (bytes <= current_remaining_) {
        char* result = pool_begin_;
        pool_begin_ += bytes;
        current_remaining_ -= bytes;
        return result;
    }

    // 大对象直接分配
    if (bytes > BIG_BYTES_THRESHOLD) {
        return doAllocate(bytes);
    }

    // 更新新的当前内存分配点位
    char* result = doAllocate(POOL_BLOCK_BYTES);
    pool_begin_ = result + bytes;
    current_remaining_ = POOL_BLOCK_BYTES - bytes;
    return result;
}

char* Allocator::allocateAligned(size_t bytes) {
    assert(bytes > 0);

    // 计算需要补齐的字节数，使内存块首地址是align的整数倍
    size_t mod = reinterpret_cast<uintptr_t>(pool_begin_) & (ALIGN - 1);
    size_t need_to_add = ALIGN - mod;
    size_t total_need = bytes + need_to_add;

    // 若有可分配内存调整可分配水位后直接返回，否则重新分配
    char* result = nullptr;
    if (total_need <= current_remaining_) {
        result = pool_begin_ + need_to_add;
        pool_begin_ += total_need;
        current_remaining_ -= total_need;
    } else if (bytes > BIG_BYTES_THRESHOLD) {
        // 重新分配new出来的内存一定是对齐的
        result = doAllocate(bytes);
    } else {
        result = doAllocate(POOL_BLOCK_BYTES);
        pool_begin_ = result + bytes;
        current_remaining_ = POOL_BLOCK_BYTES - bytes;
    }
    
    assert((reinterpret_cast<uintptr_t>(result) & (ALIGN - 1)) == 0);
    return result;
}

char* Allocator::doAllocate(size_t bytes) {
    char* result = new char[bytes];
    pool_.push_back(result);

    // 加上new时额外存储的数组尺寸
    allocated_size_.fetch_add(sizeof(char*) + bytes, std::memory_order_relaxed);
    return result;
}

}