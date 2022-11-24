/*
 * @Author: Tomato
 * @Date: 2022-01-15 22:59:21
 * @LastEditTime: 2022-11-24 09:54:34
 */
#include <gtest/gtest.h>
#include <tomato_common/io.h>
#include <random>

namespace tomato {

const std::string FILENAME_1 = "test-1";
const std::string FILENAME_2 = "test-2";

std::string writeTestFile(std::shared_ptr<AppendOnlyFile> writer, const std::string& filename) {
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

std::string readTestFile(std::shared_ptr<SequentialFile> reader, size_t size) {
    std::string read_content("");
    reader->read(size, read_content);
    return read_content;
}

TEST(POSIX_IO, write_and_read) {
    std::shared_ptr<AppendOnlyFile> writer = createAppendOnlyFile(FILENAME_1);
    EXPECT_TRUE(writer->isOpen());
    std::string content = writeTestFile(writer, FILENAME_1);

    // 测试顺序读
    std::shared_ptr<SequentialFile> reader = createSequentialFile(FILENAME_1);
    EXPECT_TRUE(reader->isOpen());
    std::string read_content = readTestFile(reader, content.size());
    EXPECT_EQ(content, read_content);

    // 测试mmap随机读
    std::shared_ptr<RandomAccessFile> random_access_reader = createRandomAccessFile(FILENAME_1);
    EXPECT_TRUE(random_access_reader->isOpen());
    std::random_device seed;
    std::minstd_rand generator(seed());
    std::uniform_int_distribution<uint64_t> distribution(0, content.size() / 2);
    for (int i = 0; i < 10; ++i) {
        uint64_t offset = distribution(generator);
        size_t remain = content.size() - offset;
        std::string output;
        random_access_reader->read(offset, remain, output);
        EXPECT_EQ(output, content.substr(offset, remain));
    }
}

TEST(POSIX_IO, skip_read_test) {
    std::shared_ptr<AppendOnlyFile> writer = createAppendOnlyFile(FILENAME_2);
    EXPECT_TRUE(writer->isOpen());
    std::string test_content = "abcdefghiafwfdsvcdsvaegeaefjk";
    OperatorResult status = writer->append(test_content);
    EXPECT_TRUE(status.isSuccess());
    status = writer->sync();
    EXPECT_TRUE(status.isSuccess());

    std::shared_ptr<SequentialFile> reader = createSequentialFile(FILENAME_2);
    ::off_t skip_size = 10;
    EXPECT_TRUE(writer->isOpen());
    status = reader->skip(skip_size);
    EXPECT_TRUE(status.isSuccess());
    std::string result;
    size_t remain_size = test_content.size() - skip_size;
    status = reader->read(test_content.size() - skip_size, result);
    EXPECT_TRUE(status.isSuccess());
    EXPECT_EQ(result, test_content.substr(skip_size, remain_size));
}

}



int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}