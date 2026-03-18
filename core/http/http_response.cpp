#include "core/http/http_response.h"

#include <fcntl.h>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

HttpResponse::HttpResponse(const std::string& staticRoot)
    : staticRoot_(staticRoot),
      mappedFile_(nullptr),
      mappedFileLen_(0) {}
void HttpResponse::setStaticRoot(const std::string& staticRoot) {
    staticRoot_ = staticRoot;
}
bool HttpResponse::buildResponse(const std::string& requestPath, bool keepAlive, Metrics& metrics) {
    unmapFile();
    header_.clear();

    metrics.incTotalRequests();

    if (requestPath == "/metrics") {
        metrics.incSuccessfulRequests();
        buildJsonResponse(metrics.toJson(), keepAlive);
        return true;
    }

    if (isPathUnsafe(requestPath)) {
        metrics.incForbiddenRequests();
        return buildFileResponse(staticRoot_+"/403.html", 403, "Forbidden", false);
    }

    std::string path = requestPath;
    if (path == "/") {
        path = "/index.html";
    }

    std::string fullPath = staticRoot_ + path;

    struct stat fileStat {};
    if (stat(fullPath.c_str(), &fileStat) < 0 || S_ISDIR(fileStat.st_mode)) {
        metrics.incNotFoundRequests();
        return buildFileResponse(staticRoot_+"/404.html", 404, "Not Found", false);
    }
    metrics.incSuccessfulRequests();
    return buildFileResponse(fullPath, 200, "OK", keepAlive);
}

const std::string& HttpResponse::header() const {
    return header_;
}

const char* HttpResponse::mappedFile() const {
    return mappedFile_;
}

size_t HttpResponse::mappedFileLen() const {
    return mappedFileLen_;
}

bool HttpResponse::hasFile() const {
    return mappedFile_ != nullptr && mappedFileLen_ > 0;
}

void HttpResponse::unmapFile() {
    if (mappedFile_ != nullptr) {
        munmap(mappedFile_, mappedFileLen_);
        mappedFile_ = nullptr;
        mappedFileLen_ = 0;
    }
}

bool HttpResponse::isPathUnsafe(const std::string& path) const {
    return path.find("..") != std::string::npos;
}

void HttpResponse::buildJsonResponse(const std::string& body, bool keepAlive) {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: application/json; charset=utf-8\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n";
    response << "\r\n";
    response << body;

    header_ = response.str();
}

bool HttpResponse::buildFileResponse(const std::string& filePath, int statusCode,
                                     const std::string& statusText, bool keepAlive) {
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
        const std::string fallback = "<html><body><h1>File Open Error</h1></body></html>";
        std::ostringstream response;
        response << "HTTP/1.1 500 Internal Server Error\r\n";
        response << "Content-Type: text/html; charset=utf-8\r\n";
        response << "Content-Length: " << fallback.size() << "\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        response << fallback;
        header_ = response.str();
        return true;
    }

    struct stat fileStat {};
    if (fstat(fd, &fileStat) < 0 || S_ISDIR(fileStat.st_mode)) {
        close(fd);
        return false;
    }

    mappedFileLen_ = static_cast<size_t>(fileStat.st_size);

    if (mappedFileLen_ > 0) {
        void* mapped = mmap(nullptr, mappedFileLen_, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);

        if (mapped == MAP_FAILED) {
            mappedFile_ = nullptr;
            mappedFileLen_ = 0;
            const std::string fallback = "<html><body><h1>mmap failed</h1></body></html>";
            std::ostringstream response;
            response << "HTTP/1.1 500 Internal Server Error\r\n";
            response << "Content-Type: text/html; charset=utf-8\r\n";
            response << "Content-Length: " << fallback.size() << "\r\n";
            response << "Connection: close\r\n";
            response << "\r\n";
            response << fallback;
            header_ = response.str();
            return true;
        }

        mappedFile_ = static_cast<char*>(mapped);
    } else {
        close(fd);
        mappedFile_ = nullptr;
    }

    std::string mimeType = getMimeType(filePath);

    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << mimeType << "; charset=utf-8\r\n";
    response << "Content-Length: " << mappedFileLen_ << "\r\n";
    response << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n";
    response << "\r\n";

    header_ = response.str();
    return true;
}

std::string HttpResponse::getMimeType(const std::string& filePath) const {
    if (filePath.size() >= 5 && filePath.substr(filePath.size() - 5) == ".html") return "text/html";
    if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".css") return "text/css";
    if (filePath.size() >= 3 && filePath.substr(filePath.size() - 3) == ".js") return "application/javascript";
    if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".png") return "image/png";
    if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".jpg") return "image/jpeg";
    if (filePath.size() >= 5 && filePath.substr(filePath.size() - 5) == ".jpeg") return "image/jpeg";
    if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".gif") return "image/gif";
    if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".svg") return "image/svg+xml";
    if (filePath.size() >= 5 && filePath.substr(filePath.size() - 5) == ".json") return "application/json";
    if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".txt") return "text/plain";
    if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".ico") return "image/x-icon";
    return "application/octet-stream";
}