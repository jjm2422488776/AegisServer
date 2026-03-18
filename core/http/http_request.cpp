#include "core/http/http_request.h"

#include <sstream>

bool HttpRequest::parse(const std::string& rawRequest) {
    method_.clear();
    path_.clear();
    version_.clear();
    headers_.clear();

    std::istringstream stream(rawRequest);
    std::string line;

    // 1. 解析请求行
    if (!std::getline(stream, line)) {
        return false;
    }

    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    if (!parseRequestLine(line)) {
        return false;
    }

    // 2. 解析请求头
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            break;
        }

        parseHeaderLine(line);
    }

    return true;
}

bool HttpRequest::parseRequestLine(const std::string& line) {
    std::istringstream lineStream(line);
    if (!(lineStream >> method_ >> path_ >> version_)) {
        return false;
    }
    return true;
}

void HttpRequest::parseHeaderLine(const std::string& line) {
    size_t pos = line.find(':');
    if (pos == std::string::npos) {
        return;
    }

    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);

    while (!value.empty() && value.front() == ' ') {
        value.erase(value.begin());
    }

    headers_[key] = value;
}

const std::string& HttpRequest::method() const {
    return method_;
}

const std::string& HttpRequest::path() const {
    return path_;
}

const std::string& HttpRequest::version() const {
    return version_;
}

std::string HttpRequest::getHeader(const std::string& key) const {
    auto it = headers_.find(key);
    if (it == headers_.end()) {
        return "";
    }
    return it->second;
}