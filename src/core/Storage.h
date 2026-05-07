#pragma once

#include <string>

#include "BookList.h"

namespace Storage {
bool initDatabaseIfNeeded(const std::string& filename, bool withDemoData);
bool loadFromFile(const std::string& filename, BookList& list);
bool saveToFile(const std::string& filename, const BookList& list);
}  // namespace Storage
