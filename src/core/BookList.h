#pragma once

#include <string>
#include <vector>

#include "../model/BookNode.h"

class BookList {
public:
    BookList();
    ~BookList();

    void clear();
    bool isEmpty() const;

    void addBook(const Book& b);
    bool deleteById(int id);
    bool updateById(int id, const Book& newBook);

    Book* findById(int id);
    std::vector<Book> findByKeyword(const std::string& key) const;

    void sortByPrice(bool asc = true);
    void sortByDate(bool asc = true);
    void sortByTitle(bool asc = true);

    std::vector<Book> toVector() const;
    void rebuildFromVector(const std::vector<Book>& arr);
    int nextId() const;

private:
    BookNode* head_;
};
