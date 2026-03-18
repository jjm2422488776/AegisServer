#pragma once

#include <string>
#include <unordered_map>

class Config {
public:
    bool loadFromFile(const std::string& filePath);

    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;

private:
    std::unordered_map<std::string, std::string> kv_;
};