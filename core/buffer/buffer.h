#pragma once

#include <string>
#include <vector>

class Buffer {
public:
    explicit Buffer(size_t initialSize = 4096);

    size_t readableBytes() const;
    size_t writableBytes() const;

    const char* peek() const;
    char* beginWrite();

    void hasWritten(size_t len);
    void retrieve(size_t len);
    void retrieveAll();

    std::string retrieveAllToString();
    void append(const char* data, size_t len);
    void append(const std::string& str);

private:
    void ensureWritable(size_t len);

private:
    std::vector<char> buffer_;
    size_t readPos_;
    size_t writePos_;
};