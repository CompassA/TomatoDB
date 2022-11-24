/*
 * @Author: Tomato
 * @Date: 2021-12-27 16:31:45
 * @LastEditTime: 2022-11-24 10:58:20
 */
#include <tomato_db/memory_table.h>

#include <cstring>
#include <climits>

namespace tomato {

/**
 * @brief 字典序排序
 * 
 * @param v1 待比较数1
 * @param v2 待比较数2–
 * @return <0 : v1 < v2;
 *         == 0: v1 == v2;
 *         >0: v1 > v2;
 */
int TableItemComparator::operator()(const TableItem& v1, const TableItem& v2) const {
    int res = memcmp(v1.key, v2.key, v1.key_len < v2.key_len ? v1.key_len : v2.key_len);
    if (res != 0) {
        return res;
    }
    if (v1.key_len < v2.key_len) {
        return -1;
    } else if (v1.key_len > v2.key_len) {
        return 1;
    }

    // key 相等的情况，比较seq
    assert(v1.seq_id != v2.seq_id);
    if (v1.seq_id > v2.seq_id) {
        return -1;
    } else {
        return 1;
    }
}

MemoryTable::MemoryTable()
    : allocator_(),
      comparator_(),
      table_(&allocator_, comparator_) {}
    
void MemoryTable::add(const uint64_t seq, ItemType type, 
                      const std::string& key, const std::string& value) {
    table_.insert(createItem(seq, type, key, value));
}

std::shared_ptr<std::string> MemoryTable::get(const std::string& key) {
    // 构建查找条件，seq传最大值(因为memtable会有多版本的值，并且以最新版本的值为准)
    TableItem item = createItem(UINT64_MAX, ItemType::VALUE, key, "");
 
    // 为找到值, 或者值被删除, 或者值与key对不上,返回空
    auto it = Table::Iterator(&table_);
    it.seek(item);
    if (!it.valid()) {
        return std::shared_ptr<std::string>(nullptr);
    }

    const TableItem& target_item = it.key();
    if (target_item.type == ItemType::DELETION) {
        return std::shared_ptr<std::string>(nullptr);
    }

    if (::memcmp(target_item.key, key.c_str(), 
            key.size() > target_item.key_len ? target_item.key_len : key.size())) {
        return std::shared_ptr<std::string>(nullptr);            
    }

    return std::make_shared<std::string>(target_item.value, target_item.value_len);
}

TableItem MemoryTable::createItem(const uint64_t seq, ItemType type, 
                                  const std::string& key, const std::string& value) {
    uint64_t key_len = 0;
    char* key_ptr = nullptr;
    if (key.size() > 0) {
        char* buffer = allocator_.allocateAligned(key.size());
        std::memcpy(buffer, key.c_str(), key.size());
        key_ptr = buffer;
        key_len = key.size();
    }

    uint64_t value_len = 0;
    char* value_ptr = nullptr;
    if (value.size() > 0) {
        char* buffer = allocator_.allocateAligned(value.size());
        std::memcpy(buffer, value.c_str(), value.size());
        value_ptr = buffer;
        value_len = value.size();
    }
    return TableItem(seq, ItemType::VALUE, key_len, key_ptr, value_len, value_ptr);
}

}