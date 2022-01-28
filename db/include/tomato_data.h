/*
 * @Author: Tomato
 * @Date: 2021-12-27 22:17:45
 * @LastEditTime: 2022-01-28 17:46:55
 */
#ifndef TOMATO_DB_DB_INCLUDE_TOMATO_DATA_H
#define TOMATO_DB_DB_INCLUDE_TOMATO_DATA_H

#include <cstdint>
#include <cstddef>

namespace tomato {

struct TableConfig {
    uint64_t block_size_threshold = 4096;
    int block_group_size = 16;
};

enum ItemType {
    /**
     * @brief 正常键值对
     * 
     */
    VALUE = 0x0,
    /**
     * @brief 删除的值
     * 
     */
    DELETION = 0x1,
};

/**
 * @brief 一个键值对，内存均是通过Allocator分配的
 * 
 */
struct TableItem {
    /**
     * @brief 数据序号，每个键值对有一个唯一的序号
     * 
     */
    uint64_t seq_id;

    /**
     * @brief 值的类型
     * 
     */
    ItemType type;

    /**
     * @brief key的长度
     * 
     */
    uint64_t key_len;

    /**
     * @brief key的值
     * 
     */
    const char*const key;

    /**
     * @brief value的长度
     * 
     */
    uint64_t value_len;

    /**
     * @brief value的值
     * 
     */
    const char*const value;

    TableItem(): seq_id(0),
                 type(ItemType::VALUE),
                 key_len(0),
                 key(nullptr), 
                 value_len(0), 
                 value(nullptr) {}

    TableItem(uint64_t seq_id_, ItemType type_,
              uint64_t key_len_, const char*const key_,
              uint64_t value_len_, const char*const value_)
            : seq_id(seq_id_),
              type(type_),
              key_len(key_len_),
              key(key_), 
              value_len(value_len_), 
              value(value_) {}

    TableItem(const TableItem&) = default;
    TableItem& operator=(const TableItem&) = delete;
};

/**
 * @brief 用于对键值对进行对比
 * 
 */
struct TableItemComparator {
    int operator()(const TableItem& v1, const TableItem& v2) const;
};


}
#endif