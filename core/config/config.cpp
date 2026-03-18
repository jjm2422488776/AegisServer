#include "core/config/config.h"

#include <fstream>
#include <sstream>

namespace {
std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')) {
        ++start;
    }

    size_t end = s.size();
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r' || s[end - 1] == '\n')) {
        --end;
    }

    return s.substr(start, end - start);
}
}

bool Config::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));
        kv_[key] = value;
    }

    return true;
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = kv_.find(key);
    if (it == kv_.end()) {
        return defaultValue;
    }
    return it->second;
}

int Config::getInt(const std::string& key, int defaultValue) const {
    auto it = kv_.find(key);
    if (it == kv_.end()) {
        return defaultValue;
    }

    try {
        return std::stoi(it->second);
    } catch (...) {
        return defaultValue;
    }
}