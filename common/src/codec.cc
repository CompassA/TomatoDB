/*
 * @Author: Tomato
 * @Date: 2021-12-22 22:06:13
 * @LastEditTime: 2022-11-24 09:06:07
 */
#include <tomato_common/codec.h>

namespace tomato {
namespace codec {

/**
 * @brief 二进制为10000000, 将一个数与此数相或,快速取得后7位数字
 * 
 */
static const int MOD_CODE = 1 << 7;

/**
 * @brief 二进制01111111
 * 
 */
static const int MASK = MOD_CODE - 1;

/**
 * @brief 对32位无符号数进行定长编码
 *
 * @param value 32位无符号数 [32...24...16...8...1] (数字表示第几个bit)
 * @return std::string 编码的结果 [8..1][16..9][24..17][32..25]
 */
std::string encodeFixed32(uint32_t value) {
    char buffer[sizeof(value)];
    uint8_t* const intBuffer = reinterpret_cast<uint8_t*>(buffer);
    intBuffer[0] = static_cast<uint8_t>(value);
    intBuffer[1] = static_cast<uint8_t>(value >> 8);
    intBuffer[2] = static_cast<uint8_t>(value >> 16);
    intBuffer[3] = static_cast<uint8_t>(value >> 24);
    return std::string(buffer, sizeof(value));
}

/**
 * @brief 取入参的前32位bit解码成32为无符号数
 * 
 * @param value 
 * @return uint32_t
 */
uint32_t decodeFixed32(const std::string& value) {
    const uint8_t* seq = reinterpret_cast<const uint8_t*>(value.c_str());
    uint32_t result = 0;
    uint32_t shift = 0;
    for (size_t i = 0; i < 4; ++i) {
        result |= (static_cast<uint32_t>(seq[i]) << shift);
        shift += 8;
    }
    return result;
}

/**
 * @brief 对64位无符号数进行定长编码
 * 
 * @param value 64位无符号数[64..56..48..42..36..28...1] (数字表示第几个bit)
 * @return std::string [8..1][16..9][24..17][32..25][40..33][48..41][56..49][64..57]
 */
std::string encodeFixed64(uint64_t value) {
    char buffer[sizeof(value)];
    uint8_t* const intBuffer = reinterpret_cast<uint8_t*>(buffer);
    intBuffer[0] = static_cast<uint8_t>(value);
    intBuffer[1] = static_cast<uint8_t>(value >> 8);
    intBuffer[2] = static_cast<uint8_t>(value >> 16);
    intBuffer[3] = static_cast<uint8_t>(value >> 24);
    intBuffer[4] = static_cast<uint8_t>(value >> 32);
    intBuffer[5] = static_cast<uint8_t>(value >> 40);
    intBuffer[6] = static_cast<uint8_t>(value >> 48);
    intBuffer[7] = static_cast<uint8_t>(value >> 56);
    return std::string(buffer, sizeof(value));
}

/**
 * @brief 取入参的前64位bit解码成64为无符号数
 * 
 * @param value 
 * @return uint32_t 
 */
uint64_t decodeFixed64(const std::string& value) {
    const uint8_t* seq = reinterpret_cast<const uint8_t*>(value.c_str());
    uint64_t result = 0;
    int shift = 0;
    for (int i = 0; i < 8; ++i) {
        // 解了很久的bug, 不cast的话，uint8的seq[i]会溢出
        result |= (static_cast<uint64_t>(seq[i]) << shift);
        shift += 8;
    }
    return result;
}

/**
 * @brief 对32位无符号数进行变长编码
 * 
 * @param value 32位无符号数 [32...24...16...8...1] (数字表示第几个bit)
 * @return std::string 每个字节第一位，标志当前字节是否是上下文中的最后一个字节(0表示是最后一位)，后七位为具体数据。
 *         [1..7|1][8..15|1][16..23|1].....[....32|0]
 */
std::string encodeVar32(uint32_t value) {
    // 32 / 7 = 4组 .... 4位, 最大需要5个字节
    char buffer[5];
    uint8_t* intBuffer = reinterpret_cast<uint8_t*>(buffer);
    while (value >= MOD_CODE) {
        *intBuffer = static_cast<uint8_t>(value | MOD_CODE);
        ++intBuffer;
        value >>= 7;
    }
    *intBuffer = static_cast<uint8_t>(value);
    return std::string(buffer, static_cast<size_t>(reinterpret_cast<char*>(intBuffer) - buffer + 1));
}

/**
 * @brief 解码变长的32位无符号数字
 * 
 * @param value 
 * @return uint32_t 
 */
std::pair<uint32_t, int> decodeVar32(const std::string& value) {
    const auto& res = decodeVar64(value);
    return std::make_pair(static_cast<uint32_t>(res.first), res.second);
}

/**
 * @brief 对64位无符号数进行变长编码
 * 
 * @param value 
 * @return std::string 
 */
std::string encodeVar64(uint64_t value) {
    // 64 / 7 = 9组 ... 1位, 最多需要10个字节
    char buffer[10];
    uint8_t* intBuffer = reinterpret_cast<uint8_t*>(buffer);
    while (value >= MOD_CODE) {
        *intBuffer = static_cast<uint8_t>(value | MOD_CODE);
        ++intBuffer;
        value >>= 7;
    }
    *intBuffer = static_cast<uint8_t>(value);
    return std::string(buffer, static_cast<size_t>(reinterpret_cast<char*>(intBuffer) - buffer + 1));
}

/**
 * @brief 解码变长的64位无符号数字
 * 
 * @param value 
 * @return uint64_t 
 */
std::pair<uint64_t, int> decodeVar64(const std::string& value) {
    const char* begin = value.c_str();
    const uint8_t* seq = reinterpret_cast<const uint8_t*>(begin);

    // 遍历每个字节直到当前字节首位为0
    uint64_t result = 0;
    uint8_t cur = *seq;
    int shift = 0;
    while (cur & MOD_CODE) {
        result |= (static_cast<uint64_t>(cur & MASK) << shift);
        cur = *(++seq);
        shift += 7;
    }
    result |= static_cast<uint64_t>(cur & MASK) << shift;
    return std::make_pair(result, reinterpret_cast<const char*>(seq) - begin + 1);
}




}
}