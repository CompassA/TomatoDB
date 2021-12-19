/*
 * @Author: Tomato
 * @Date: 2021-12-18 23:51:23
 * @LastEditTime: 2021-12-19 22:47:59
 */
#ifndef TOMATODB_COMMON_INCLUDE_TOMATO_SKIP_LIST_H
#define TOMATODB_COMMON_INCLUDE_TOMATO_SKIP_LIST_H

#include <random>
#include <string>
#include <atomic>
#include <cassert>

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
    bool contains(const Value& value);

    /**
     * @brief 删除一个值
     * 
     * @param value 要删除的值
     */
    void remove(const Value& value);

    /**
     * @brief 将跳表可视化
     * 
     */
    std::string showSelf();
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
    Node* searchFirstNotLess(const Value& target);

    /**
     * @brief 查询跳表中第一个不小于target元素的节点, 并记录搜索路径
     * 
     * @param target 目标节点
     * @param path [out] 搜索路径
     * @return 查询结果
     */
    Node* searchFirstNotLess(const Value& target, Node** path);

    /**
     * @brief 获取当前跳表的最大层数
     * 
     * @return 层数[1, MAX_LEVEL] 
     */
    int getCurrentMaxLevel();

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
      comparator_(comparator),
      max_level_(1)
{
    for (int i = 0; i < MAX_LEVEL; ++i) {
        head_->setNext(i, nullptr);
    }
}

template<typename Value, typename Comparator>
void SkipList<Value, Comparator>::insert(const Value& value) {
    // 获取搜索路径
    Node* path[MAX_LEVEL];
    Node* result = searchFirstNotLess(value, path);

    // todo 不插入重复元素
    // assert(result && !comparator_(result->val, value));

    // 获取随机层高
    int newNodeLevel = randomLevel();

    // 创建节点
    Node* insertNode = newNode(value, newNodeLevel);
    int oldMaxLevel = getCurrentMaxLevel();

    // 更新链表索引与最大层高
    for (int i = 0; i < oldMaxLevel; ++i) {
        insertNode->setNext(i, path[i]->next(i));
        path[i]->setNext(i, insertNode);
    }
    if (oldMaxLevel < newNodeLevel) {
        setCurrentMaxLevel(newNodeLevel);
        for (int i = oldMaxLevel; i < newNodeLevel; ++i) {
            insertNode->setNext(i, nullptr);
            head_->setNext(i, insertNode);
        }
    }
}

template<typename Value, typename Comparator>
bool SkipList<Value, Comparator>::contains(const Value& value) {
    Node* result = searchFirstNotLess(value);
    return result && (comparator_(result->val, value) == 0);
}

template<typename Value, typename Comparator>
void SkipList<Value, Comparator>::remove(const Value& value) {
    Node* path[MAX_LEVEL];
    Node* result = searchFirstNotLess(value, path);
    // 没找到节点，不用删除
    if (result == nullptr || comparator_(result->val, value) != 0) {
        return;
    }

    for (int level = 0; level < MAX_LEVEL; ++level) {
        Node* next = result->next(level);
        if (!next) {
            int nodeLevel = level + 1 - 1;
            // 如果当前删除的节点的层数等于最大层数，更新最大层数
            if (nodeLevel == getCurrentMaxLevel()) {
                int cnt = 0;
                for (int i = 0; i < MAX_LEVEL; ++i) {
                    if (head_->next(i)) {
                        cnt++;
                    } else {
                        break;
                    }
                }
                setCurrentMaxLevel(cnt);
            }
            break;
        }
        path[level]->setNext(level, next);
    }

}

template<typename Value, typename Comparator>
std::string SkipList<Value, Comparator>::showSelf() {
    std::string result("");
    for (int i = getCurrentMaxLevel()-1; i >= 0; --i) {
        Node* cur = head_->next(i);
        if (cur == nullptr) {
            continue;
        }
        result.append("head -> ");
        while (cur) {
            result.append(cur->val + " -> ");
            cur = cur->next(i);
        }
        result.append("nullptr\n");
    }
    return result;
}

template<typename Value, typename Comparator>
inline int SkipList<Value, Comparator>::getCurrentMaxLevel() {
    return max_level_.load(std::memory_order_relaxed);
}

template<typename Value, typename Comparator>
inline void SkipList<Value, Comparator>::setCurrentMaxLevel(int level) {
    return max_level_.store(level, std::memory_order_relaxed);
}

template<typename Value, typename Comparator>
int SkipList<Value, Comparator>::randomLevel() {
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(RANDOM_MIN, RANDOM_MAX);
    int level = 1;
    int target = RANDOM_MAX / 2;
    while (distribution(generator) <= target && level < MAX_LEVEL) {
        ++level;

        // 加层数的概率逐步下降
        target = static_cast<int>(target * 0.8);
    }
    return level;
}

template<typename Value, typename Comparator>
typename SkipList<Value, Comparator>::Node* 
SkipList<Value, Comparator>::newNode(const Value& value, int level) {
    char* const m = allocator_->allocateAligned(
        sizeof(Node) + sizeof(std::atomic<Node*>) * level);
    return new (m) Node(value);
}

template<typename Value, typename Comparator>
typename SkipList<Value, Comparator>::Node* 
SkipList<Value, Comparator>::searchFirstNotLess(const Value& target) {
    return searchFirstNotLess(target, nullptr);
}

template<typename Value, typename Comparator>
typename SkipList<Value, Comparator>::Node* 
SkipList<Value, Comparator>::searchFirstNotLess(const Value& target, Node** path) {
    Node* cur = head_;
    assert(cur != nullptr);

    // 从高到低遍历层数
    for (int level = getCurrentMaxLevel()-1; level >= 0; --level) {
        // 找到当前层最后一个小于目标节点的元素
        Node* curNext = cur->next(level);
        while (curNext && comparator_(curNext->val, target) < 0) {
            cur = curNext;
            curNext = cur->next(level);
        }

        // 记录路径
        if (path) {
            path[level] = cur;
        }
    }
    return cur->next(0);
}


template<typename Value, typename Comparator>
SkipList<Value, Comparator>::Node::Node(const Value& value): val(value) {
}


}

#endif