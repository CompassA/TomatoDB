/*
 * @Author: Tomato
 * @Date: 2022-01-06 23:42:04
 * @LastEditTime: 2023-02-01 22:14:04
 */
#include <tomato_common/codec.h>
#include <tomato_db/table_meta.h>
#include <tomato_db/sstable_builder.h>
#include <gtest/gtest.h>

namespace tomato {

TEST(SSTABLE, blockBuilderAdd) {
    TableConfig config;
    config.block_group_size = 2;
    BlockBuilder builder(config);
    const std::string& key1 = "key1";
    const std::string& value1 = "value1  bbbb";
    const std::string& key2 = "key2";
    const std::string& value2 = "value2aaaaaa";
    const std::string& key3 = "key3";
    const std::string& value3 = "value3awefwaegv dfag ";
    const std::string& content = builder.getContent();
    std::string expect;
    expect.append(codec::encodeVar64(0));
    expect.append(codec::encodeVar64(key1.size()));
    expect.append(codec::encodeVar64(value1.size()));
    expect.append(key1);
    expect.append(value1);
    builder.add(key1, value1);
    EXPECT_EQ(content, expect);
    expect.append(codec::encodeVar64(3));
    expect.append(codec::encodeVar64(1));
    expect.append(codec::encodeVar64(value1.size()));
    expect.append(key2.c_str() + 3);
    expect.append(value2);
    builder.add(key2, value2);
    EXPECT_EQ(content, expect);
    expect.append(codec::encodeVar64(0));
    expect.append(codec::encodeVar64(key3.size()));
    expect.append(codec::encodeVar64(value3.size()));
    expect.append(key3);
    expect.append(value3);
    builder.add(key3, value3);
    EXPECT_EQ(content, expect);
}



}