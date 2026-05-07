#include "Validator.h"

#include <cctype>

bool Validator::isValidDate(const std::string& date) {
    if (date.size() != 10) {
        return false;
    }
    for (size_t i = 0; i < date.size(); ++i) {
        if (i == 4 || i == 7) {
            if (date[i] != '-') {
                return false;
            }
        } else if (!std::isdigit(static_cast<unsigned char>(date[i]))) {
            return false;
        }
    }
    return true;
}

bool Validator::isValidPrice(const std::string& price) {
    if (price.empty()) {
        return false;
    }
    bool dotSeen = false;
    for (char ch : price) {
        if (ch == '.') {
            if (dotSeen) {
                return false;
            }
            dotSeen = true;
        } else if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

bool Validator::isNonEmpty(const std::string& s) {
    for (char ch : s) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            return true;
        }
    }
    return false;
}
