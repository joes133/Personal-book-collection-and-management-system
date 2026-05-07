#pragma once

#include "Book.h"

struct BookNode {
    Book data;
    BookNode* next = nullptr;

    explicit BookNode(const Book& b) : data(b), next(nullptr) {}
};
