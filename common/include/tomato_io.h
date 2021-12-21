/*
 * @Author: Tomato
 * @Date: 2021-12-22 00:04:56
 * @LastEditTime: 2021-12-22 00:40:20
 */
#include <string>

namespace tomato {

/**
 * @brief 操作状态码
 * 
 */
enum Code {
    SUCCESS = 0,
};

/**
 * @brief IO操作结果
 * 
 */
class OperatorResult {
public:
    Code getCode() const {
        return code_;
    }

private:
    Code code_;
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
    virtual ~AppendOnlyFile();

    /**
     * @brief 将数据追加到文件末尾
     * 
     * @param data 要写入文件的数据 
     * @return OperatorResult 
     */
    virtual OperatorResult append(const std::string& data) = 0;

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
    virtual ~SequentialFile();
    
    /**
     * @brief 从文件上次的offset开始，顺序读取n个字节
     * 
     * @param size 要读取的字节数
     * @param output [out] 将数据读取到这块内存中
     * @return OperatorResult 操作结果
     */
    virtual OperatorResult read(size_t size, std::string* output) = 0;

    /**
     * @brief 从文件当前偏移位置开始，跳过n个字节
     * 
     * @param size 要跳过的字节数
     * @return OperatorResult 
     */
    virtual OperatorResult skip(size_t size) = 0;
};

}