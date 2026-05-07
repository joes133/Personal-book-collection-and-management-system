#include "StringUtil.h"

#include <cctype>
#include <sstream>

std::vector<std::string> StringUtil::split(const std::string& s, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        parts.push_back(item);
    }
    return parts;
}

std::string StringUtil::trim(const std::string& s) {
    size_t left = 0;
    while (left < s.size() && std::isspace(static_cast<unsigned char>(s[left])) != 0) {
        ++left;
    }

    size_t right = s.size();
    while (right > left && std::isspace(static_cast<unsigned char>(s[right - 1])) != 0) {
        --right;
    }

    return s.substr(left, right - left);
}
