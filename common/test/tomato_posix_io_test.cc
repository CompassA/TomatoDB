/*
 * @Author: Tomato
 * @Date: 2022-01-15 22:59:21
 * @LastEditTime: 2022-01-16 17:00:26
 */
#include <gtest/gtest.h>
#include <tomato_io.h>
#include <random>

namespace tomato {

const std::string filename = "test-1";

std::string writeTestFile(AppendOnlyFile* writer) {
    std::string content = "";

    std::random_device seed;
    std::minstd_rand generator(seed());
    std::uniform_int_distribution<int> distribution(1, 255);

    // 大块写入
    for (int i = 0; i < 4000000; ++i) {
        content.push_back(distribution(generator));
    }
    writer->append(content);

    // 碎片写入
    std::string small_str("");
    for (int i = 0; i < 3000000; ++i) {
        small_str.push_back(static_cast<char>(distribution(generator)));
        writer->append(small_str.substr(i, 1));
    }
    writer->sync();
    content.append(small_str);
    return content;
}

std::string readTestFile(SequentialFile* reader, size_t size) {
    std::string read_content("");
    reader->read(size, read_content);
    return read_content;
}

TEST(POSIX_IO, write_and_sequential_read) {
    AppendOnlyFile* writer = createAppendOnlyFile(filename);
    EXPECT_TRUE(writer->isOpen());
    std::string content = writeTestFile(writer);

    SequentialFile* reader = createSequentialFile(filename);
    EXPECT_TRUE(reader->isOpen());
    std::string read_content = readTestFile(reader, content.size());

    EXPECT_EQ(content, read_content);
    delete writer;
    delete reader;
}

}



int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}