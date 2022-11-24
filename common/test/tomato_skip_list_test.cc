/*
 * @Author: Tomato
 * @Date: 2021-12-19 11:28:21
 * @LastEditTime: 2022-11-24 09:07:24
 */
#include <gtest/gtest.h>
#include <tomato_common/allocator.h>
#include <tomato_common/skip_list.h>
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

void printLevel(const SkipList<Key, Comparator>& list) {
    for (int i = list.getCurrentMaxLevel(); i > 0; --i) {
        std::vector<Key> levelElements = list.getLevel(i-1);
        if (levelElements.empty()) {
            break;
        }
        ::printf("level %d, size=%lu: [", i, levelElements.size());
        for (size_t j = 0; j < levelElements.size(); ++j) {
            ::printf("%llu%s", levelElements[j], j + 1 != levelElements.size() ? "->" : "]\n");
        }
    }
    ::printf("\n");
}

// 参考leveldb插入测试
TEST(SKIP_LIST_TEST, InsertAndLookup) {
    const int N = 5000;
    const int R = 10000;
    std::set<Key> keys;
    Allocator allocator;
    Comparator cmp;

    std::default_random_engine generator;
    std::uniform_int_distribution<Key> distribution(0, R);
    SkipList<Key, Comparator> list(&allocator, cmp);
    for (int i = 0; i < N; i++) {
        Key key = (distribution(generator)) % R;
        if (keys.insert(key).second) {
            list.insert(key);
        }
    }
    for (int i = 0; i < list.getCurrentMaxLevel(); ++i) {
        std::vector<Key> levelElements = list.getLevel(i);
        if (levelElements.empty() || levelElements.size() < 2) {
            break;
        }
        for (int i = 1; i < levelElements.size(); ++i) {
            EXPECT_LT(levelElements[i-1], levelElements[i]);
        }
    }

    for (int i = 0; i <= R; i++) {
        if (list.contains(i)) {
            ASSERT_EQ(keys.count(i), 1);
            list.remove(i);
            // printLevel(list);
        } else {
            ASSERT_EQ(keys.count(i), 0);
        }
    }

    EXPECT_EQ(list.getCurrentMaxLevel(), 0);
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

TEST(SKIP_LIST_TEST, iterator) {
    Allocator allocator;
    Comparator cmp;
    SkipList<Key, Comparator> list(&allocator, cmp);

    std::vector<int> values;
    for (int i = 0; i < 100000; ++i) {
        values.push_back(i);
        list.insert(i);
    }

    int index = 0;
    auto it = SkipList<Key, Comparator>::Iterator(&list);
    for (it.seekToFirst(); it.valid(); it.next()) {
        EXPECT_EQ(values[index++], it.key());
    }
    EXPECT_EQ(index, values.size());

    it = SkipList<Key, Comparator>::Iterator(&list);
    Key target = values.size()/2;
    it.seek(target);
    EXPECT_TRUE(cmp(target,it.key()) == 0);
}

} // namespace tomato


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}