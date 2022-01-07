/*
 * @Author: Tomato
 * @Date: 2022-01-05 23:30:54
 * @LastEditTime: 2022-01-07 23:48:36
 */

#include "../include/tomato_sstable_builder.h"

#include <tomato_codec.h>
#include <algorithm>

namespace tomato {

BlockBuilder::BlockBuilder(const TableConfig& config)
    : contents_(""),
      last_key_(""),
      restarts_(),
      group_size_(config.block_group_size),
      current_group_size_(0)
    {}

BlockBuilder::~BlockBuilder() {

}

void BlockBuilder::add(const std::string&key, const std::string& value) {    
    // 计算和前一个key相同的前缀字符长度
    uint64_t shared = 0;
    if (current_group_size_ == group_size_) {
        group_size_ = 0;
        restarts_.push_back(contents_.size());
    } else {
        size_t min_size = std::min(key.size(), last_key_.size());
        for (size_t i = 0; i < min_size; ++i) {
            if (key[i] == last_key_[i]) {
                ++shared;
            } else {
                break;
            }
        }
    }
    
    // 编码规则 与前面key共享的字节数(64bit) + key非共享字节数(64bit) + value的长度(64bit) + key + value
    uint64_t unshared = key.size() - shared;
    contents_.append(codec::encodeVar64(shared));
    contents_.append(codec::encodeVar64(unshared));
    contents_.append(codec::encodeVar64(value.size()));
    contents_.append(key.c_str() + shared, unshared);
    contents_.append(value);

    last_key_.resize(shared);
    last_key_.append(key.c_str() + shared, unshared);

    ++current_group_size_;
}

void BlockBuilder::reset() {

}

const std::string& BlockBuilder::getContent() {
    const std::string& res = contents_;
    return res;
}


}