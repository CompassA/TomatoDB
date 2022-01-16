/*
 * @Author: Tomato
 * @Date: 2021-12-22 00:04:56
 * @LastEditTime: 2022-01-16 17:01:34
 */
#ifndef TOMATODB_COMMON_INCLUDE_TOMATO_IO_H
#define TOMATODB_COMMON_INCLUDE_TOMATO_IO_H

#include <string>

namespace tomato {

/**
 * @brief 操作状态码
 * 
 */
enum Status {
    SUCCESS = 0,
    FAILURE = 1,
};

/**
 * @brief IO操作结果
 * 
 */
class OperatorResult {
public:
    OperatorResult(int code, std::string msg = ""): code_(code), message_(std::move(msg)) {
        if (code == 0) {
            status_ = Status::SUCCESS;
        } else {
            status_ = Status::FAILURE;
        }
    }

    Status getStatus() const {
        return status_;
    }

    bool isSuccess() const {
        return code_ == 0;
    }
    int getCode() const {
        return code_;
    }
public:
    static OperatorResult success() {
        return OperatorResult(0);
    }
private:
    int code_;
    Status status_;
    std::string message_;
};

/**
 * @brief 追加写文件
 * 
 */
class AppendOnlyFile {
public:
    AppendOnlyFile() = default;
    AppendOnlyFile(const AppendOnlyFile&) = delete;
    AppendOnlyFile& operator=(const AppendOnlyFile&) = delete;
    virtual ~AppendOnlyFile() = default;

    /**
     * @brief 是否打开
     * 
     * @return true 可操作
     * @return false 文件无法操作
     */
    virtual bool isOpen() = 0;

    /**
     * @brief 将数据追加到文件末尾
     * 
     * @param data 要写入文件的数据 
     * @return OperatorResult 
     */
    virtual OperatorResult append(const std::string& data) = 0;

    /**
     * @brief 将缓冲区写入page-cache
     * 
     */
    virtual OperatorResult flush() = 0;
    
    /**
     * @brief 将数据落盘
     * 
     * @return OperatorResult 
     */
    virtual OperatorResult sync() = 0;

    /**
     * @brief 关闭文件，并将缓冲区中未落盘的文件落盘
     * 
     * @return OperatorResult 
     */
    virtual OperatorResult close() = 0;

    /**
     * @brief 得到文件名(得到的是构造文件时传入的值)
     * 
     * @return std::string 
     */
    virtual std::string getFileName() = 0;

    /**
     * @brief 得到文件夹名(根据构造函数的传入情况，若构造函数传了个相对路径，则只会返回相对路径)
     * 
     * @return std::string 
     */
    virtual std::string getDirName() = 0;
};

/**
 * @brief 顺序读文件
 * 
 */
class SequentialFile {
public:
    SequentialFile() = default;
    SequentialFile(const SequentialFile&) = delete;
    SequentialFile& operator=(const SequentialFile&) = delete;
    virtual ~SequentialFile() = default;
    
    /**
     * @brief 是否打开
     * 
     * @return true 可操作
     * @return false 文件无法操作
     */
    virtual bool isOpen() = 0;
    
    /**
     * @brief 从文件上次的offset开始，顺序读取n个字节
     * 
     * @param size 要读取的字节数
     * @param output [out] 将数据读取到这块内存中
     * @return OperatorResult 操作结果
     */
    virtual OperatorResult read(size_t size, std::string& output) = 0;

    /**
     * @brief 从文件当前偏移位置开始，跳过n个字节
     * 
     * @param size 要跳过的字节数
     * @return OperatorResult 
     */
    virtual OperatorResult skip(off_t size) = 0;

    /**
     * @brief 关闭文件
     * 
     * @return OperatorResult 
     */
    virtual OperatorResult close() = 0;

    /**
     * @brief 得到文件名(得到的是构造文件时传入的值)
     * 
     * @return std::string 
     */
    virtual std::string getFileName() = 0;

    /**
     * @brief 得到文件夹名(根据构造函数的传入情况，若构造函数传了个相对路径，则只会返回相对路径)
     * 
     * @return std::string 
     */
    virtual std::string getDirName() = 0;
};

/**
 * @brief 创建顺序写文件
 * 
 * @return AppendOnlyFile* 
 */
AppendOnlyFile* createAppendOnlyFile(const std::string&);

/**
 * @brief 创建顺序读文件
 * 
 * @return SequentialFile* 
 */
SequentialFile* createSequentialFile(const std::string&);

}

#endif