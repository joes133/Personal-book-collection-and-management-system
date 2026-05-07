#pragma once

#include <string>
#include <vector>

namespace StringUtil {
std::vector<std::string> split(const std::string& s, char delimiter);
std::string trim(const std::string& s);
}  // namespace StringUtil
