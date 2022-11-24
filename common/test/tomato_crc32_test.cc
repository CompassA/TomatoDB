/*
 * @Author: Tomato
 * @Date: 2021-12-26 16:17:51
 * @LastEditTime: 2022-11-24 09:07:07
 */
#include <tomato_common/crc32.h>
#include <gtest/gtest.h>

namespace tomato {

TEST(CRC, StandardResults) {
    // From rfc3720 section B.4.
    char buf[32];

    memset(buf, 0, sizeof(buf));
    ASSERT_EQ(0x8a9136aa, crc32(buf, sizeof(buf)));

    memset(buf, 0xff, sizeof(buf));
    ASSERT_EQ(0x62a8ab43, crc32(buf, sizeof(buf)));

    for (int i = 0; i < 32; i++) {
        buf[i] = i;
    }
    ASSERT_EQ(0x46dd794e, crc32(buf, sizeof(buf)));

    for (int i = 0; i < 32; i++) {
        buf[i] = 31 - i;
    }
    ASSERT_EQ(0x113fdb5c, crc32(buf, sizeof(buf)));

    uint8_t data[48] = {
        0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x18, 0x28, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    ASSERT_EQ(0xd9963a56, crc32(reinterpret_cast<char*>(data), sizeof(data)));
}

TEST(CRC, Values) {
    ASSERT_NE(crc32("a", 1), crc32("foo", 3));
}

}


int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}