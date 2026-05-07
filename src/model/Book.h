#pragma once

#include <string>

struct Book {
    int id = 0;
    std::string title;
    std::string category;
    std::string buyDate;  // YYYY-MM-DD
    double price = 0.0;
    std::string intro;
};
