#pragma once

#include <string>
#include <unordered_map>

class HttpRequest {
public:
    bool parse(const std::string& rawRequest);

    const std::string& method() const;
    const std::string& path() const;
    const std::string& version() const;
    std::string getHeader(const std::string& key) const;

private:
    bool parseRequestLine(const std::string& line);
    void parseHeaderLine(const std::string& line);

private:
    std::string method_;
    std::string path_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
};