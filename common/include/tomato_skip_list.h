/*
 * @Author: Tomato
 * @Date: 2021-12-18 23:51:23
 * @LastEditTime: 2021-12-27 16:07:29
 */
#ifndef TOMATODB_COMMON_INCLUDE_TOMATO_SKIP_LIST_H
#define TOMATODB_COMMON_INCLUDE_TOMATO_SKIP_LIST_H

#include <random>
#include <string>
#include <atomic>
#include <cassert>
#include <vector>
#include <ctime>

#include "tomato_allocator.h"

namespace tomato {

/**
 * @brief 跳表
 * 
 * @tparam Value 跳表存储的值
 * @tparam Comparator 跳表存储的值的比较方式
 */
template<typename Value, typename Comparator>
class SkipList {
public:
    class Node;
public:
    SkipList(Allocator* allocator, Comparator comparator);
    
    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

    /**
     * @brief 从跳表中插入一个值
     * 
     * @param value 要插入的值
     */
    void insert(const Value& value);

    /**
     * @brief 查询值是否存在
     * 
     * @param value 要查询的值
     * @return true 存在; false 不存在
     */
    bool contains(const Value& value) const;

    /**
     * @brief 删除一个值
     * 
     * @param value 要删除的值
     */
    void remove(const Value& value);

    /**
     * @brief 获取当前跳表的最大层数
     * 
     * @return 层数[1, MAX_LEVEL] 
     */
    int getCurrentMaxLevel() const;

    /**
     * @brief 获取一层的全部元素
     * 
     * @param level 
     * @return std::vector<Value> 
     */
    std::vector<Value> getLevel(int level) const;
private:
    // 内存分配器
    Allocator* const allocator_;

    // 跳表头节点
    Node* const head_;

    // 跳表节点层高最大值
    std::atomic<int> max_level_;

    // value间的比较方式, 需要重载括号运算符
    const Comparator comparator_;
private:
    enum {
        // 最大层高
        MAX_LEVEL = 10,
        // 随机数最大边界
        RANDOM_MAX = 100,
        // 随机数最小边界
        RANDOM_MIN = 1
    };

    /**
     * @brief 获取一个随机的层高
     * 
     * @return 层高
     */
    int randomLevel();

    /**
     * @brief 创建一个新的节点
     * 
     * @param value 节点值
     * @param level 节点有几层
     * @return Node* 
     */
    Node* newNode(const Value& value, int level);

    /**
     * @brief 查询跳表中第一个不小于target元素的节点
     * 
     * @param target 目标节点
     * @return 查找结果 
     */
    Node* searchFirstNotLess(const Value& target) const;

    /**
     * @brief 查询跳表中第一个不小于target元素的节点, 并记录搜索路径
     * 
     * @param target 目标节点
     * @param path [out] 搜索路径
     * @return 查询结果
     */
    Node* searchFirstNotLess(const Value& target, std::vector<Node*>* path) const;

    /**
     * @brief Set the Current Max Level 
     * 
     * @param level 
     */
    void setCurrentMaxLevel(int level);
};

template<typename Value, typename Comparator>
class SkipList<Value, Comparator>::Node {
public:
    Node(const Value& value);

    /**
     * @brief 获取下一个节点，加了内存屏障，防止内存分配与构造函数初始化重排序
     * 
     * @param level 层数
     * @return Node* 目标节点
     */
    Node* next(int level) {
        assert(level >= 0);
        return next_[level].load(std::memory_order_acquire);
    }

    /**
     * @brief 指定层数，设置当前节点的下一个节点
     * 
     * @param level 层数
     * @param node 要设置的下一个节点
     */
    void setNext(int level, Node* node) {
        assert(level >= 0);
        next_[level].store(node, std::memory_order_release);
    }

public:
    const Value val;
private:
    std::atomic<Node*> next_[];
};


template<typename Value, typename Comparator>
SkipList<Value, Comparator>::SkipList(Allocator* allocator, Comparator comparator)
    : allocator_(allocator),
      head_(newNode(0, MAX_LEVEL)),
      max_level_(0),
      comparator_(comparator)
{
    for (int i = 0; i < MAX_LEVEL; ++i) {
        head_->setNext(i, nullptr);
    }
}

template<typename Value, typename Comparator>
void SkipList<Value, Comparator>::insert(const Value& value) {
    // 获取搜索路径
    std::vector<Node*> path(MAX_LEVEL, nullptr);
    Node* result = searchFirstNotLess(value, &path);

    // 不插入重复元素
    assert(result == nullptr || comparator_(result->val, value) != 0);

    // 获取随机层高
    int new_node_level = randomLevel();

    // 创建节点
    Node* insert_node = newNode(value, new_node_level);

    // 更新链表索引与最大层高
    for (int i = 0; i < new_node_level; ++i) {
        Node* path_node = (path[i] == nullptr ? head_ : path[i]);
        insert_node->setNext(i, path_node->next(i));
        path_node->setNext(i, insert_node);
    }

    // 更新层高
    int old_max_level = getCurrentMaxLevel();
    if (old_max_level < new_node_level) {
        setCurrentMaxLevel(new_node_level);
    }
}

template<typename Value, typename Comparator>
bool SkipList<Value, Comparator>::contains(const Value& value) const {
    Node* result = searchFirstNotLess(value);
    return result && (comparator_(result->val, value) == 0);
}

template<typename Value, typename Comparator>
void SkipList<Value, Comparator>::remove(const Value& value) {
    std::vector<Node*> path(MAX_LEVEL, nullptr);
    Node* result = searchFirstNotLess(value, &path);
    // 没找到节点，不用删除
    if (result == nullptr || comparator_(result->val, value) != 0) {
        return;
    }

    // 遍历前缀节点，直到前缀节点为空或不是当前节点的前缀
    for (int level = 0; level <= MAX_LEVEL; ++level) {
        Node* prev = level == MAX_LEVEL ? nullptr: path[level];
        if (!prev || !prev->next(level) || comparator_(prev->next(level)->val, value)) {
            int current_max_level = getCurrentMaxLevel();
            // path[level]为null时，level即为当前节点层数
            // 通过便利头节点的方式统计最高层数
            if (level == current_max_level) {
                int new_max_level = 0;
                for (int i = 0; i < MAX_LEVEL; ++i) {
                    if (head_->next(i)) {
                        ++new_max_level;
                    } else {
                        break;
                    }
                }
                setCurrentMaxLevel(new_max_level);
            }
            break;
        } 
        // 节点相同, 删除节点, 调整索引
        else {
            path[level]->setNext(level, result->next(level));
        } 
    }

}

template<typename Value, typename Comparator>
std::vector<Value> SkipList<Value, Comparator>::getLevel(int level) const {
    std::vector<Value> result(0);
    Node* cur = head_->next(level);
    while (cur) {
        result.push_back(cur->val);
        cur = cur->next(level);
    }
    return result;
}

template<typename Value, typename Comparator>
inline int SkipList<Value, Comparator>::getCurrentMaxLevel() const {
    return max_level_.load(std::memory_order_relaxed);
}

template<typename Value, typename Comparator>
inline void SkipList<Value, Comparator>::setCurrentMaxLevel(int level) {
    return max_level_.store(level, std::memory_order_relaxed);
}

template<typename Value, typename Comparator>
int SkipList<Value, Comparator>::randomLevel() {
    std::random_device seed;
    std::minstd_rand generator(seed());
    std::uniform_int_distribution<int> distribution(RANDOM_MIN, RANDOM_MAX);
    int level = 1;
    int target = RANDOM_MAX / 2;
    int random_val = distribution(generator);
    while (random_val <= target && level < MAX_LEVEL) {
        ++level;
        random_val = distribution(generator);
    }
    return level;
}

template<typename Value, typename Comparator>
typename SkipList<Value, Comparator>::Node* 
SkipList<Value, Comparator>::newNode(const Value& value, int level) {
    char* const m = allocator_->allocateAligned(
        sizeof(Node) + sizeof(std::atomic<Node*>) * level);
    Node* result = new (m) Node(value);
    for (int i = 0; i < level; ++i) {
        result->setNext(i, nullptr);
    }
    return result;
}

template<typename Value, typename Comparator>
typename SkipList<Value, Comparator>::Node* 
SkipList<Value, Comparator>::searchFirstNotLess(const Value& target) const {
    return searchFirstNotLess(target, nullptr);
}

template<typename Value, typename Comparator>
typename SkipList<Value, Comparator>::Node* 
SkipList<Value, Comparator>::searchFirstNotLess(const Value& target, std::vector<Node*>* path) const {
    Node* cur = head_;
    assert(cur != nullptr);

    // 从高到低遍历层数
    for (int level = getCurrentMaxLevel()-1; level >= 0; --level) {
        // 找到当前层最后一个小于目标节点的元素
        Node* cur_next = cur->next(level);
        while (cur_next && comparator_(cur_next->val, target) < 0) {
            cur = cur_next;
            cur_next = cur->next(level);
        }

        // 记录路径
        if (path) {
            (*path)[level] = cur;
        }
    }
    return cur->next(0);
}


template<typename Value, typename Comparator>
SkipList<Value, Comparator>::Node::Node(const Value& value): val(value) {
}


}

#endif