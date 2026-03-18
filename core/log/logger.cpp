#include "core/log/logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::info(const std::string& msg) {
    log("INFO", msg);
}

void Logger::warn(const std::string& msg) {
    log("WARN", msg);
}

void Logger::error(const std::string& msg) {
    log("ERROR", msg);
}

void Logger::log(const std::string& level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::system_clock::now();
    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);

    std::tm tmNow {};
    localtime_r(&timeNow, &tmNow);

    std::ostringstream oss;
    oss << "[" << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S") << "]";
    oss << "[" << level << "] " << msg << "\n";

    std::cout << oss.str();
}