#include "core/http/http_conn.h"
#include <sys/uio.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

HttpConn::HttpConn()
    : fd_(-1),
      keepAlive_(false),
      mappedBytesSent_(0),
      processing_(false),
      processDone_(false) {}

HttpConn::HttpConn(int fd)
    : fd_(fd),
      keepAlive_(false),
      mappedBytesSent_(0),
      processing_(false),
      processDone_(false) {}

void HttpConn::init(int fd, const std::string& clientIp, const std::string& staticRoot) {
    fd_ = fd;
    clientIp_ = clientIp;
    keepAlive_ = false;
    mappedBytesSent_ = 0;
    processing_ = false;
    processDone_ = false;
    readBuffer_.retrieveAll();
    writeBuffer_.retrieveAll();
    response_.unmapFile();
    response_.setStaticRoot(staticRoot);
}

int HttpConn::fd() const {
    return fd_;
}

const std::string& HttpConn::clientIp() const {
    return clientIp_;
}

bool HttpConn::read() {
    char buf[4096];
    while (true) {
        ssize_t n = recv(fd_, buf, sizeof(buf), 0);
        if (n > 0) {
            readBuffer_.append(buf, static_cast<size_t>(n));
        } else if (n == 0) {
            return false;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            return false;
        }
    }
    return readBuffer_.readableBytes() > 0;
}

bool HttpConn::process(Metrics& metrics) {
    std::string rawRequest(readBuffer_.peek(), readBuffer_.readableBytes());

    if (!request_.parse(rawRequest)) {
        const std::string body = "<html><body><h1>400 Bad Request</h1></body></html>";
        std::string response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        writeBuffer_.append(response);
        keepAlive_ = false;
        mappedBytesSent_ = 0;
        return true;
    }

    std::string connection = request_.getHeader("Connection");
    keepAlive_ = (connection == "keep-alive" || connection == "Keep-Alive");

    if (request_.method() != "GET") {
        const std::string body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        std::string response =
            "HTTP/1.1 405 Method Not Allowed\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        writeBuffer_.append(response);
        keepAlive_ = false;
        mappedBytesSent_ = 0;
        return true;
    }

    if (!response_.buildResponse(request_.path(), keepAlive_, metrics)) {
        return false;
    }

    writeBuffer_.append(response_.header());
    mappedBytesSent_ = 0;
    readBuffer_.retrieveAll();
    return true;
}

bool HttpConn::write() {
    while (writeBuffer_.readableBytes() > 0 || (response_.hasFile() && mappedBytesSent_ < response_.mappedFileLen())) {
        struct iovec iov[2];
        int iovcnt = 0;

        // iov[0]: header 未发送部分
        if (writeBuffer_.readableBytes() > 0) {
            iov[iovcnt].iov_base = const_cast<char*>(writeBuffer_.peek());
            iov[iovcnt].iov_len = writeBuffer_.readableBytes();
            ++iovcnt;
        }

        // iov[1]: file 未发送部分
        if (response_.hasFile() && mappedBytesSent_ < response_.mappedFileLen()) {
            iov[iovcnt].iov_base = const_cast<char*>(response_.mappedFile() + mappedBytesSent_);
            iov[iovcnt].iov_len = response_.mappedFileLen() - mappedBytesSent_;
            ++iovcnt;
        }

        ssize_t n = writev(fd_, iov, iovcnt);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return true;
            }
            return false;
        }

        size_t bytesSent = static_cast<size_t>(n);

        // 优先消耗 header
        if (writeBuffer_.readableBytes() > 0) {
            size_t headerBytes = writeBuffer_.readableBytes();
            if (bytesSent >= headerBytes) {
                writeBuffer_.retrieve(headerBytes);
                bytesSent -= headerBytes;
            } else {
                writeBuffer_.retrieve(bytesSent);
                bytesSent = 0;
            }
        }

        // 剩余部分再消耗 file
        if (bytesSent > 0 && response_.hasFile() && mappedBytesSent_ < response_.mappedFileLen()) {
            mappedBytesSent_ += bytesSent;
        }
    }

    return true;
}

bool HttpConn::isKeepAlive() const {
    return keepAlive_;
}

void HttpConn::reset() {
    readBuffer_.retrieveAll();
    writeBuffer_.retrieveAll();
    keepAlive_ = false;
    mappedBytesSent_ = 0;
    response_.unmapFile();
    processing_ = false;
    processDone_ = false;
}
void HttpConn::markProcessing(bool value) {
    processing_ = value;
}

bool HttpConn::isProcessing() const {
    return processing_.load();
}

void HttpConn::markProcessDone(bool value) {
    processDone_ = value;
}

bool HttpConn::isProcessDone() const {
    return processDone_.load();
}