/*
 * @Author: Tomato
 * @Date: 2021-12-24 20:34:10
 * @LastEditTime: 2022-11-24 09:06:57
 */
#include <gtest/gtest.h>
#include <tomato_common/codec.h>
#include <vector>

// 参考leveldb的数字编码单测
namespace tomato { namespace codec {


TEST(Coding, Fixed32) {
  std::string s;
  for (uint32_t v = 0; v < 300000; v++) {
    s.append(encodeFixed32(v));
  }

  for (uint32_t v = 0; v < 300000; v++) {
    uint32_t actual = decodeFixed32(s);
    ASSERT_EQ(v, actual);
    s = s.substr(4, s.length());
  }
}

TEST(Coding, Fixed64) {
  std::string s;
  for (int power = 0; power <= 63; power++) {
    uint64_t v = static_cast<uint64_t>(1) << power;
    s.append(encodeFixed64(v - 1));
    s.append(encodeFixed64(v + 0));
    s.append(encodeFixed64(v + 1));
  }

  for (int power = 0; power <= 63; power++) {
    uint64_t v = static_cast<uint64_t>(1) << power;
    uint64_t actual;
    actual = decodeFixed64(s);
    ASSERT_EQ(v - 1, actual);
    s = s.substr(8, s.length());

    actual = decodeFixed64(s);
    ASSERT_EQ(v + 0, actual);
    s = s.substr(8, s.length());

    actual = decodeFixed64(s);
    ASSERT_EQ(v + 1, actual);
    s = s.substr(8, s.length());
  }
}

TEST(Coding, Varint32) {
  std::string s;
  for (uint32_t i = 0; i < (32 * 32); i++) {
    uint32_t v = (i / 32) << (i % 32);
    s.append(encodeVar32(v));
  }

  for (uint32_t i = 0; i < (32 * 32); i++) {
    uint32_t expected = (i / 32) << (i % 32);
    std::pair<uint32_t, int> res = decodeVar32(s);
    ASSERT_EQ(expected, res.first);
    ASSERT_EQ(res.first, decodeVar32(s.substr(0, 0 + res.second)).first);
    s = s.substr(res.second, s.length());
  }
  ASSERT_EQ(0, s.length());
}

TEST(Coding, Varint64) {
  // Construct the list of values to check
  std::vector<uint64_t> values;
  // Some special values
  values.push_back(0);
  values.push_back(100);
  values.push_back(~static_cast<uint64_t>(0));
  values.push_back(~static_cast<uint64_t>(0) - 1);
  for (uint32_t k = 0; k < 64; k++) {
    // Test values near powers of two
    const uint64_t power = 1ull << k;
    values.push_back(power);
    values.push_back(power - 1);
    values.push_back(power + 1);
  }

  std::string s;
  for (size_t i = 0; i < values.size(); i++) {
    s.append(encodeVar64(values[i]));
  }

  for (size_t i = 0; i < values.size(); i++) {
    std::pair<uint64_t, int> res = decodeVar64(s);
    ASSERT_EQ(res.first, values[i]);
    ASSERT_EQ(res.first, decodeVar64(s.substr(0, 0 + res.second)).first);
    s = s.substr(res.second, s.length());
  }
  ASSERT_EQ(0, s.length());
}



}}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
