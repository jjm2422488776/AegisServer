#pragma once

#include "core/metrics/metrics.h"

#include <string>

class HttpResponse {
public:
    explicit HttpResponse(const std::string& staticRoot = "static");

    void setStaticRoot(const std::string& staticRoot);

    bool buildResponse(const std::string& requestPath, bool keepAlive, Metrics& metrics);

    const std::string& header() const;
    const char* mappedFile() const;
    size_t mappedFileLen() const;
    bool hasFile() const;

    void unmapFile();

private:
    bool isPathUnsafe(const std::string& path) const;
    void buildJsonResponse(const std::string& body, bool keepAlive);
    bool buildFileResponse(const std::string& filePath, int statusCode,
                           const std::string& statusText, bool keepAlive);
    std::string getMimeType(const std::string& filePath) const;

private:
    std::string staticRoot_;
    std::string header_;
    char* mappedFile_;
    size_t mappedFileLen_;
};