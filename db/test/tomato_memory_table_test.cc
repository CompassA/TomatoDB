/*
 * @Author: Tomato
 * @Date: 2021-12-27 19:44:09
 * @LastEditTime: 2021-12-27 22:12:33
 */
#include <tomato_memory_table.h>
#include <gtest/gtest.h>

namespace tomato {

TEST(MEMORY_TABLE, empty) {
    MemoryTable table;
    std::shared_ptr<std::string> res = table.get("dummy");
    EXPECT_TRUE(!res);
}

TEST(MEMORY_TABLE, addAndGet) {
    MemoryTable table;
    std::string test_key = "key";
    std::string test_val = "val";
    uint64_t seq = 1;
    table.add(seq, ItemType::VALUE, test_key, test_val);

    std::shared_ptr<std::string> res = table.get(test_key);
    EXPECT_TRUE(res);
    EXPECT_EQ(test_val, *res);
}

TEST(MEMORY_TABLE, multiVersionAddAndGet) {
    MemoryTable table;
    std::string test_key = "key";
    std::string test_val = "val";
    std::string test_val2 = "val2";
    uint64_t seq = 1;
    table.add(seq, ItemType::VALUE, test_key, test_val);
    table.add(seq+1, ItemType::VALUE, test_key, test_val2);

    std::shared_ptr<std::string> res = table.get(test_key);
    EXPECT_TRUE(res);
    EXPECT_EQ(test_val2, *res);
}


}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}