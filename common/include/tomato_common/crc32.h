/*
 * @Author: Tomato
 * @Date: 2021-12-26 16:17:31
 * @LastEditTime: 2021-12-26 16:42:59
 */
#ifndef TOMATODB_COMMON_INCLUDE_TOMATO_CRC32_H
#define TOMATODB_COMMON_INCLUDE_TOMATO_CRC32_H

#include <cstdint>
#include <cstddef>

namespace tomato {

/**
 * @brief 输入一串字节，计算其crc校验
 * 
 * @param seq 字节串
 * @param length 多少个字节
 * @return uint32_t crc32校验和
 */
uint32_t crc32(const char* seq, size_t length);

}

#endif