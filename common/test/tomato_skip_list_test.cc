/*
 * @Author: Tomato
 * @Date: 2021-12-19 11:28:21
 * @LastEditTime: 2021-12-19 17:12:27
 */
#include <gtest/gtest.h>
#include <tomato_allocator.h>
#include <tomato_skip_list.h>
#include <atomic>
#include <vector>

namespace tomato {

// 测试内存分配
TEST(SKIP_LIST_TEST, node_size_test) {
    using NodeType = SkipList<long, char*>::Node;
    Allocator allocator;
    
    const int level = 10;
    size_t bytes = sizeof(NodeType) + sizeof(std::atomic<NodeType*>) * level;
    char* const m = allocator.allocateAligned(bytes);   
    long val = 1024; 
    NodeType* node = new (m) NodeType(val);
    std::vector<NodeType*> children;
    for (int i = 0; i < level; ++i) {
        char* const m = allocator.allocateAligned(bytes);
        NodeType* n = new (m) NodeType(val);
        children.push_back(n);
        node->setNext(i, n);
    }


    EXPECT_EQ(val, node->val);
    for (int i = 0; i < level; ++i) {
        EXPECT_EQ(node->next(i), children[i]);
    }
}

} // namespace tomato


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}