/*
 * @Author: Tomato
 * @Date: 2021-12-27 16:19:21
 * @LastEditTime: 2022-11-24 10:02:14
 */
#ifndef TOMATO_DB_DB_INCLUDE_TOMATO_MEMORY_TABLE_H
#define TOMATO_DB_DB_INCLUDE_TOMATO_MEMORY_TABLE_H

#include <tomato_db/table_meta.h>
#include <tomato_common/allocator.h>
#include <tomato_common/skip_list.h>

#include <string>
#include <memory>

namespace tomato {



/**
 * @brief 内存表
 * 
 */
class MemoryTable {
public:
    using Table = SkipList<TableItem, TableItemComparator>;
public:
    MemoryTable();
    MemoryTable(const MemoryTable&) = delete;
    MemoryTable& operator=(const MemoryTable&) = delete;
    ~MemoryTable() = default;

    /**
     * @brief 向内存表添加一个键值对
     * 
     * @param seq 序列号
     * @param type 键值对类型
     * @param key 键
     * @param value 值
     */
    void add(const uint64_t seq, ItemType type, 
             const std::string& key, const std::string& value);

    /**
     * @brief 查找内存表
     * 
     * @param key 键
     * @return std::shared_ptr<std::string> 被智能指针包裹的值, 若未查找到值, 返回空的智能指针
     */
    std::shared_ptr<std::string> get(const std::string& key);

private:
    /**
     * @brief 向内存表添加一个键值对
     * 
     * @param seq 序列号
     * @param type 键值对类型
     * @param key 键
     * @param value 值
     */
    TableItem createItem(const uint64_t seq, ItemType type,
                         const std::string& key, const std::string& value);

private:
    /**
     * @brief 保存在内存表中的内存数据均由内存分配器分配
     * 
     */
    Allocator allocator_;

    /**
     * @brief 键值对比较方式
     * 
     */
    const TableItemComparator comparator_;

    /**
     * @brief 内存键值对表
     * 
     */
    Table table_;
};

}

#endif