/*
 * @Author: Tomato
 * @Date: 2021-12-18 13:27:47
 * @LastEditTime: 2023-02-01 22:54:06
 */
#include <gtest/gtest.h>
#include <tomato_common/allocator.h>

#include <random>
#include <ctime>

namespace tomato {

// 参考下leveldb关于Arena的单测
TEST(ALLOCATOR_TEST, allocator_aligned_test) {
    std::vector<std::pair<size_t, char*>> allocated;
    Allocator allocator;
    size_t bytes = 0;

    std::default_random_engine rnd(time(0));
    std::uniform_int_distribution<unsigned> u(1, 6000);

    // 分配500次内存, 每次分配的尺寸随机, 测试对齐分配的尺寸是否大于大于等于目标尺寸
    for (int i = 0; i < 500; i++) {
        size_t s = u(rnd);
        char* r = allocator.allocateAligned(s);

        for (size_t b = 0; b < s; b++) {
            r[b] = static_cast<char>(i % 256);
        }

        bytes += s;
        allocated.push_back(std::make_pair(s, r));
        ASSERT_GE(allocator.getAllocatedSize(), bytes);
    }

    // 测试分配内存后写入的值是否正确
    for (size_t i = 0; i < allocated.size(); i++) {
        size_t num_bytes = allocated[i].first;
        const char* p = allocated[i].second;
        for (size_t b = 0; b < num_bytes; b++) {
            ASSERT_EQ(int(p[b]) & 0xff, i % 256);
        }
    }
}

TEST(ALLOCATOR_TEST, allocate_test) {
    std::default_random_engine rnd;
    std::uniform_int_distribution<unsigned> u(1, 4000);
    std::vector<std::pair<size_t, char*>> allocated;
    size_t usedBytes = 0;
    Allocator allocator;

    for (int i = 0; i < 500; ++i) {
        size_t needToBorrow = u(rnd);
        char* r = allocator.allocate(needToBorrow);
        usedBytes += needToBorrow;
        allocated.push_back(std::make_pair(needToBorrow, r));
        
        for (size_t b = 0; b < needToBorrow; b++) {
            r[b] = i % 256;
        }
        ASSERT_GE(allocator.getAllocatedSize(), usedBytes);
    }

    for (size_t i = 0; i < allocated.size(); i++) {
        size_t num_bytes = allocated[i].first;
        const char* p = allocated[i].second;
        for (size_t b = 0; b < num_bytes; b++) {
            ASSERT_EQ(int(p[b]) & 0xff, i % 256);
        }
    }
}

}