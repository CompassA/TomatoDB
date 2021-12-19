/*
 * @Author: Tomato
 * @Date: 2021-12-18 23:51:23
 * @LastEditTime: 2021-12-19 17:15:07
 */
#ifndef TOMATODB_COMMON_INCLUDE_TOMATO_SKIP_LIST_H
#define TOMATODB_COMMON_INCLUDE_TOMATO_SKIP_LIST_H

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
    bool contains(const Value& valuex);

    /**
     * @brief 删除一个值
     * 
     * @param value 要删除的值
     */
    void remove(const Value& value);
private:
    // 跳表头节点
    Node* const head_;

    // 内存分配器
    Allocator* const allocator_;

    // 跳表节点层高最大值
    std::atomic<int> max_level_;

    // value间的比较方式, 需要重载括号运算符
    const Comparator comparator_;
private:
    // 最大层高
    enum {
        MAX_LEVEL = 10
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
     * @param level 
     * @return Node* 
     */
    Node* newNode(const Value& value, const int level);

    /**
     * @brief 查询跳表中第一个不小于target元素的节点
     * 
     * @param target 目标节点
     * @return 查找结果 
     */
    Node* searchFirstNotLess(const Value* target);

    /**
     * @brief 查询跳表中第一个不小于target元素的节点, 并记录搜索路径
     * 
     * @param target 目标节点
     * @param path [out] 搜索路径
     * @return 查询结果
     */
    Node* searchFirstNotLess(const Value* target, Node** path);
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
    : head_(newNode(nullptr, MAX_LEVEL)),
      allocator_(allocator),
      max_level_(1),
      comparator_(comparator) 
{
    for (int i = 0; i < MAX_LEVEL; ++i) {
        head_->setNext(i, nullptr);
    }
}

template<typename Value, typename Comparator>
void SkipList<Value, Comparator>::insert(const Value& value) {

}

template<typename Value, typename Comparator>
bool SkipList<Value, Comparator>::contains(const Value& valuex) {
    
}

template<typename Value, typename Comparator>
void SkipList<Value, Comparator>::remove(const Value& value) {

}

template<typename Value, typename Comparator>
int SkipList<Value, Comparator>::randomLevel() {

}

template<typename Value, typename Comparator>
typename SkipList<Value, Comparator>::Node* 
SkipList<Value, Comparator>::newNode(const Value& value, const int level) {
    char* const m = allocator_->allocateAligned(
        sizeof(Node) + sizeof(std::atomic<Node*>) * level);
    return new (m) Node(value);
}

template<typename Value, typename Comparator>
typename SkipList<Value, Comparator>::Node* 
SkipList<Value, Comparator>::searchFirstNotLess(const Value* target) {

}

template<typename Value, typename Comparator>
typename SkipList<Value, Comparator>::Node* 
SkipList<Value, Comparator>::searchFirstNotLess(const Value* target, Node** path) {

}


template<typename Value, typename Comparator>
SkipList<Value, Comparator>::Node::Node(const Value& value): val(value) {
}


}

#endif