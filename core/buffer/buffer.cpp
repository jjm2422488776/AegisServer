#include "core/buffer/buffer.h"

#include <algorithm>

Buffer::Buffer(size_t initialSize)
    : buffer_(initialSize),
      readPos_(0),
      writePos_(0) {}

size_t Buffer::readableBytes() const {
    return writePos_ - readPos_;
}

size_t Buffer::writableBytes() const {
    return buffer_.size() - writePos_;
}

const char* Buffer::peek() const {
    return buffer_.data() + readPos_;
}

char* Buffer::beginWrite() {
    return buffer_.data() + writePos_;
}

void Buffer::hasWritten(size_t len) {
    writePos_ += len;
}

void Buffer::retrieve(size_t len) {
    if (len >= readableBytes()) {
        retrieveAll();
        return;
    }
    readPos_ += len;
}

void Buffer::retrieveAll() {
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::retrieveAllToString() {
    std::string result(peek(), readableBytes());
    retrieveAll();
    return result;
}

void Buffer::append(const char* data, size_t len) {
    ensureWritable(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const std::string& str) {
    append(str.data(), str.size());
}

void Buffer::ensureWritable(size_t len) {
    if (writableBytes() >= len) {
        return;
    }
    buffer_.resize(writePos_ + len);
}