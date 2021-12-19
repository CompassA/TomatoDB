/*
 * @Author: Tomato
 * @Date: 2021-12-19 11:28:21
 * @LastEditTime: 2021-12-19 21:53:47
 */
#include <gtest/gtest.h>
#include <tomato_allocator.h>
#include <tomato_skip_list.h>
#include <atomic>
#include <vector>
#include <set>

namespace tomato {

typedef uint64_t Key;

struct Comparator {
  int operator()(const Key& a, const Key& b) const {
    if (a < b) {
      return -1;
    } else if (a > b) {
      return +1;
    } else {
      return 0;
    }
  }
};

// 参考leveldb插入测试
TEST(SKIP_LIST_TEST, InsertAndLookup) {
    const int N = 2000;
    const int R = 5000;
    std::set<Key> keys;
    Allocator allocator;
    Comparator cmp;

    std::default_random_engine generator;
    std::uniform_int_distribution<Key> distribution(0, 8000000000);
    SkipList<Key, Comparator> list(&allocator, cmp);
    for (int i = 0; i < N; i++) {
        Key key = (distribution(generator)) % R;
        if (keys.insert(key).second) {
            list.insert(key);
        }
    }

    ::printf("%s", list.showSelf().c_str());

    for (int i = 0; i < R; i++) {
        if (list.contains(i)) {
            ASSERT_EQ(keys.count(i), 1);
            list.remove(i);
        } else {
            ASSERT_EQ(keys.count(i), 0);
        }
    }
}

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
        char* const d = allocator.allocateAligned(bytes);
        NodeType* n = new (d) NodeType(val);
        children.push_back(n);
        node->setNext(i, n);
    }


    EXPECT_EQ(val, node->val);
    for (int i = 0; i < level; ++i) {
        EXPECT_EQ(reinterpret_cast<uintptr_t>(node->next(i)), reinterpret_cast<uintptr_t>(children[i]));
    }
}

} // namespace tomato


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}