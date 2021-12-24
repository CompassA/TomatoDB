/*
 * @Author: Tomato
 * @Date: 2021-12-22 21:48:44
 * @LastEditTime: 2021-12-24 21:56:15
 */
#ifndef TOMATODB_COMMON_INCLUDE_TOMATO_CODEC_H
#define TOMATODB_COMMON_INCLUDE_TOMATO_CODEC_H

#include <string>
#include <utility>

namespace tomato {
namespace codec {

/**
 * @brief 对32位无符号数进行定长编码
 * 
 * @param value 
 * @return std::string 
 */
std::string encodeFixed32(uint32_t value);

/**
 * @brief 取入参的前32位bit解码成32为无符号数
 * 
 * @param value 
 * @return uint32_t
 */
uint32_t decodeFixed32(const std::string& value);

/**
 * @brief 对64位无符号数进行定长编码
 * 
 * @param value 
 * @return std::string 
 */
std::string encodeFixed64(uint64_t value);

/**
 * @brief 取入参的前64位bit解码成64为无符号数
 * 
 * @param value 
 * @return uint32_t 
 */
uint64_t decodeFixed64(const std::string& value);

/**
 * @brief 对32位无符号数进行变长编码
 * 
 * @param value 
 * @return std::string 
 */
std::string encodeVar32(uint32_t value);

/**
 * @brief 解码变长的32位无符号数字
 * 
 * @param value 
 * @return [result, offset]
 */
std::pair<uint32_t, int> decodeVar32(const std::string& value);

/**
 * @brief 对64位无符号数进行变长编码
 * 
 * @param value 
 * @return std::string 
 */
std::string encodeVar64(uint64_t value);

/**
 * @brief 解码变长的64位无符号数字
 * 
 * @param value 
 * @return uint64_t [result, offset]
 */
std::pair<uint64_t, int> decodeVar64(const std::string& value);

}
}

#endif