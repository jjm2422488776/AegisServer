#pragma once

#include "core/buffer/buffer.h"
#include "core/http/http_request.h"
#include "core/http/http_response.h"
#include "core/metrics/metrics.h"

#include <atomic>
#include <string>

class HttpConn {
public:
    HttpConn();
    explicit HttpConn(int fd);

    void init(int fd, const std::string& clientIp, const std::string& staticRoot);
    int fd() const;
    const std::string& clientIp() const;

    bool read();
    bool process(Metrics& metrics);
    bool write();

    bool isKeepAlive() const;
    void reset();

    void markProcessing(bool value);
    bool isProcessing() const;

    void markProcessDone(bool value);
    bool isProcessDone() const;

private:
    int fd_;
    std::string clientIp_;
    Buffer readBuffer_;
    Buffer writeBuffer_;
    HttpRequest request_;
    HttpResponse response_;
    bool keepAlive_;
    size_t mappedBytesSent_;

    std::atomic<bool> processing_;
    std::atomic<bool> processDone_;
};