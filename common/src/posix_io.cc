/*
 * @Author: Tomato
 * @Date: 2022-01-14 22:25:32
 * @LastEditTime: 2022-11-24 08:43:31
 */

#include <tomato_common/io.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include <algorithm>
#include <cstring>
#include <cerrno>

namespace tomato {

const size_t BUFFER_SIZE = 65536;

std::string findDirName(const std::string& filename) {
    auto pos = filename.rfind('/');
    if (pos == std::string::npos) {
        return std::string(".");
    }
    return filename.substr(0, pos);
}

uint64_t getFileSize(const std::string& filename) {
    struct ::stat file_stat;
    if (::stat(filename.c_str(), &file_stat) != 0) {
        return 0;
    }
    return file_stat.st_size;
}

class PosixAppendOnlyFile final : public AppendOnlyFile {
public:
    PosixAppendOnlyFile(std::string filename)
        : buffer_pos_(0), filename_(std::move(filename)), dirname_(findDirName(filename)) {
        fd_ = ::open(filename_.c_str(), O_TRUNC | O_WRONLY | O_CREAT | 0, 0644);
    }

    ~PosixAppendOnlyFile() override {
        if (isOpen()) {
            close();
        }
    }

    OperatorResult append(const std::string& data) override {
        const char* data_ptr = data.c_str();
        size_t unwritten_size = data.size();

        // 如果缓冲区剩余空间足够，写入缓冲区即可
        size_t writable_size = std::min(unwritten_size, BUFFER_SIZE - buffer_pos_);
        std::memcpy(buffer_ + buffer_pos_, data_ptr, writable_size);
        buffer_pos_ += writable_size;
        data_ptr += writable_size;
        unwritten_size -= writable_size;
        if (unwritten_size == 0) {
            return OperatorResult::success();
        }

        // 清空缓冲区，并将剩余小容量文件写入缓存，
        // 若为大文件，直接写入磁盘
        OperatorResult flush_result = flush();
        if (!flush_result.isSuccess()) {
            return flush_result;
        }
        if (unwritten_size < BUFFER_SIZE) {
            std::memcpy(buffer_, data_ptr, unwritten_size);
            buffer_pos_ = unwritten_size;
            return OperatorResult::success();
        }
        return systemWrite(data_ptr, unwritten_size);
    }

    OperatorResult flush() override {
        OperatorResult status = systemWrite(buffer_, buffer_pos_);
        // 写入成功时将offset置0，否则保留状态
        if (status.isSuccess()) {
            buffer_pos_ = 0; 
        }
        return status;
    }

    OperatorResult sync() override {
        // 清空用户态缓存
        OperatorResult status = flush();
        if (!status.isSuccess()) {
            return status;
        }
        // page-cache写回
        if (::fsync(fd_) < 0) {
            return OperatorResult(errno, "fsync error, filename: " + filename_);
        }
        return OperatorResult::success();
    }

    OperatorResult close() override {
        OperatorResult status = flush();
        if (::close(fd_) < 0) {
            status = OperatorResult(errno, "close file error, filename: " + filename_);
        }
        fd_ = -1;
        return status;
    }

    bool isOpen() const override {
        return fd_ >= 0;
    }

    std::string getFileName() const override {
        return filename_;
    }

    std::string getDirName() const override {
        return dirname_;
    }
private:
    /**
     * @brief 将数据写入page cache
     * 
     * @param data_ptr 数据x
     * @param unwritten_size 数据长度
     * @return OperatorResult 操作信息
     */
    OperatorResult systemWrite(const char* data_ptr, size_t unwritten_size) {
        while (unwritten_size > 0) {
            ::ssize_t written_size = ::write(fd_, data_ptr, unwritten_size);
            if (written_size < 0) {
                if (errno == EINTR) {
                    continue;
                } else {
                    return OperatorResult(errno, "file write error, file name:" + filename_);
                }
            } 
            unwritten_size -= static_cast<size_t>(written_size);
            data_ptr += written_size;
        }
        return OperatorResult::success();
    }
private:
    /**
     * @brief 文件描述符
     * 
     */
    int fd_;
    /**
     * @brief 缓冲区索引
     * 
     */
    size_t buffer_pos_;

    /**
     * @brief 缓冲区
     * 
     */
    char buffer_[BUFFER_SIZE];

    /**
     * @brief 文件名
     * 
     */
    const std::string filename_;

    /**
     * @brief 文件夹名
     * 
     */
    const std::string dirname_;
};

class PosixSequentialFile final : public SequentialFile {
public:
    PosixSequentialFile(std::string filename)
        : filename_(std::move(filename)), dirname_(findDirName(filename)) {
        fd_ = ::open(filename_.c_str(), O_RDONLY | 0);
    }

    ~PosixSequentialFile() override {
        if (fd_ >= 0) {
            close();
        }
    }

    OperatorResult read(size_t size, std::string& output) override {
        size_t unreaded_size = size;
        char buffer[BUFFER_SIZE];
        while(unreaded_size > 0) {
            ::ssize_t readed_size = ::read(fd_, buffer, BUFFER_SIZE);
            if (readed_size < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return OperatorResult(errno, "read file error, filename: " + filename_);
            }
            unreaded_size -= static_cast<size_t>(readed_size);
            output.append(buffer, static_cast<size_t>(readed_size));
        }
        return OperatorResult::success();
    }

    OperatorResult skip(::off_t size) override {
        if (::lseek(fd_, size, SEEK_CUR) < 0) {
            return OperatorResult(errno, "lseek error, filename: " + filename_);
        }
        return OperatorResult::success();
    }

    OperatorResult close() override {
        if (::close(fd_) < 0) {
            return OperatorResult(errno, "close file error, filename: " + filename_);
        }
        fd_ = -1;
        return OperatorResult::success();
    }

    std::string getFileName() const override {
        return filename_;
    }

    std::string getDirName() const override {
        return dirname_;
    }

    bool isOpen() const override {
        return fd_ >= 0;
    }
private:
    int fd_;
    std::string filename_;
    std::string dirname_;
};

class PosixRandomAccessFile final : public RandomAccessFile {
public:
    PosixRandomAccessFile(std::string filename)
        : filename_(std::move(filename)), dirname_(findDirName(filename_)), mmap_base_(nullptr), file_size_(getFileSize(filename_)) {
        fd_ = ::open(filename_.c_str(), O_RDONLY | 0);
        if (fd_ < 0) {
            return;
        }
        void* mmap_base = ::mmap(nullptr, file_size_, PROT_READ, MAP_SHARED, fd_, 0);
        if (mmap_base == MAP_FAILED) {
            ::close(fd_);
            return;
        }
        mmap_base_ = static_cast<char*>(mmap_base);
    }

    ~PosixRandomAccessFile() override {
        if (isOpen()) {
            close();
        }
    }

    OperatorResult read(uint64_t offset, size_t size, std::string& output) override {
        if (offset + size > file_size_) {
            // EINVAL参数错误
            return OperatorResult(EINVAL, "size over limit");
        }
        output.append(mmap_base_ + offset, size);
        return OperatorResult::success();
    }

    std::string getFileName() const override {
        return filename_;
    }

    std::string getDirName() const override {
        return dirname_;
    }

    bool isOpen() const override {
        return fd_ >= 0 && mmap_base_ != nullptr;
    }
    
    OperatorResult close() override {
        if (::munmap(static_cast<void*>(mmap_base_), file_size_) < 0) {
            return OperatorResult(errno, "file munmap error, filename: " + filename_);
        }
        if (::close(fd_) < 0) {
            return OperatorResult(errno, "file close error, filename: " + filename_);
        }
        fd_ = -1;
        mmap_base_ = nullptr;
        return OperatorResult::success();
    }
private:
    int fd_;
    const std::string filename_;
    const std::string dirname_;
    char* mmap_base_;
    const uint64_t file_size_;
};

std::shared_ptr<AppendOnlyFile> createAppendOnlyFile(const std::string& filename) {
    return std::make_shared<PosixAppendOnlyFile>(filename);
}

std::shared_ptr<SequentialFile> createSequentialFile(const std::string& filename) {
    return std::make_shared<PosixSequentialFile>(filename);
}

std::shared_ptr<RandomAccessFile> createRandomAccessFile(const std::string& filename) {
    return std::make_shared<PosixRandomAccessFile>(filename);
}

}