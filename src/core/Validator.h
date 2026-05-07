#pragma once

#include <string>

namespace Validator {
bool isValidDate(const std::string& date);   // YYYY-MM-DD
bool isValidPrice(const std::string& price); // non-negative number
bool isNonEmpty(const std::string& s);
}  // namespace Validator
