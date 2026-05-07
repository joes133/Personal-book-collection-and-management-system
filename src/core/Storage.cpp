#include "Storage.h"

#include <direct.h>
#include <errno.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

#include "../model/Book.h"
#include "../utils/StringUtil.h"

/*
 Storage.cpp
 -----------------------------------------
 该模块负责图书数据的持久化：
 1) 初始化数据库文件（可选写入演示数据）
 2) 从文本文件加载到链表
 3) 将链表完整写回文本文件
 
 文件格式：
 id|title|category|buyDate|price|intro
 
 编码策略：
 文件统一使用 UTF-8。
 演示数据中中文文本通过宽字符转 UTF-8 写入，避免源码编码引发编译问题。
 */

namespace {
// 解析一行文本为 Book，非法行由调用方跳过。
bool parseBookLine(const std::string& line, Book& outBook) {
    // 按分隔符拆分字段，字段数必须严格匹配。
    std::vector<std::string> fields = StringUtil::split(line, '|');
    if (fields.size() != 6) {
        return false;
    }

    try {
        // 这里仅做基础类型转换，不做业务层面的合法性约束。
        outBook.id = std::stoi(fields[0]);
        outBook.title = fields[1];
        outBook.category = fields[2];
        outBook.buyDate = fields[3];
        outBook.price = std::stod(fields[4]);
        outBook.intro = fields[5];
    } catch (...) {
        return false;
    }
    return true;
}

// 宽字符串转 UTF-8，供含中文演示数据写盘使用。
std::string toUtf8(const wchar_t* ws) {
    // 演示数据中有中文常量，这里统一转 UTF-8 再落盘。
    if (ws == nullptr || ws[0] == L'\0') {
        return "";
    }
    int len = WideCharToMultiByte(CP_UTF8, 0, ws, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) {
        return "";
    }
    std::string out(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws, -1, &out[0], len, nullptr, nullptr);
    if (!out.empty() && out.back() == '\0') {
        out.pop_back();
    }
    return out;
}
}  // namespace

// 数据库初始化：文件不存在时创建，并可按需写入演示数据。
bool Storage::initDatabaseIfNeeded(const std::string& filename, bool withDemoData) {
    // 文件已存在，说明数据库已经初始化过，直接返回。
    std::ifstream ifs(filename);
    if (ifs.is_open()) {
        return true;
    }

    // 处理 "data/books.txt" 这类路径时，先保证父目录存在。
    // 当前实现支持一层目录，课程项目场景已足够。
    const size_t pos = filename.find_last_of("\\/");
    if (pos != std::string::npos) {
        const std::string dir = filename.substr(0, pos);
        if (!dir.empty()) {
            const int mk = _mkdir(dir.c_str());
            if (mk != 0 && errno != EEXIST) {
                return false;
            }
        }
    }

    // 新建文件默认使用文本模式，便于直接查看内容。
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        return false;
    }

    if (withDemoData) {
        // 演示数据故意混合中英文，覆盖编码与界面显示场景。
        struct DemoBook {
            std::string title;
            std::string category;
            std::string date;
            double price;
            std::string intro;
        };

        const DemoBook demos[] = {
            // 保持题材多样：计算机、AI、历史、文学、心理、经济等。
            {"The C Programming Language", "Computer Science", "2026-03-01", 89.0, "Kernighan and Ritchie classic"},
            {"Introduction to Algorithms", "Computer Science", "2026-03-02", 119.0, "CLRS algorithm textbook"},
            {"Clean Code", "Software Engineering", "2026-03-03", 79.0, "Code quality and best practices"},
            {toUtf8(L"\u6D3B\u7740"), toUtf8(L"\u6587\u5B66"), "2026-03-04", 45.0, toUtf8(L"\u4F59\u534E\u4EE3\u8868\u4F5C")},
            {"Operating System Concepts", "Computer Science", "2026-03-05", 108.0, "OS fundamentals"},
            {toUtf8(L"\u8BA1\u7B97\u673A\u7F51\u7EDC"), toUtf8(L"\u6559\u6750"), "2026-03-06", 88.0, toUtf8(L"\u8C22\u5E0C\u4EC1\u6559\u6750")},
            {"The Pragmatic Programmer", "Software Engineering", "2026-03-07", 68.0, "Pragmatic development guide"},
            {"Deep Learning", "Artificial Intelligence", "2026-03-08", 128.0, "Goodfellow AI reference"},
            {toUtf8(L"\u4EBA\u5DE5\u667A\u80FD\uFF1A\u4E00\u79CD\u73B0\u4EE3\u65B9\u6CD5"), toUtf8(L"\u4EBA\u5DE5\u667A\u80FD"), "2026-03-09", 136.0, toUtf8(L"AIMA\u4E2D\u6587\u7248")},
            {"Guns, Germs, and Steel", "History", "2026-03-10", 58.0, "World history perspective"},
            {toUtf8(L"\u4EBA\u7C7B\u7B80\u53F2"), toUtf8(L"\u5386\u53F2"), "2026-03-11", 66.0, toUtf8(L"\u5C24\u74E6\u5C14\u8D6B\u62C9\u5229")},
            {"The Selfish Gene", "Biology", "2026-03-12", 54.0, "Evolutionary biology classic"},
            {toUtf8(L"\u65F6\u95F4\u7B80\u53F2"), toUtf8(L"\u7269\u7406"), "2026-03-13", 62.0, toUtf8(L"\u970D\u91D1\u79D1\u666E\u8457\u4F5C")},
            {"The Feynman Lectures on Physics", "Physics", "2026-03-14", 148.0, "Foundational physics lectures"},
            {"Thinking, Fast and Slow", "Psychology", "2026-03-15", 64.0, "Behavioral decision making"},
            {toUtf8(L"\u5F71\u54CD\u529B"), toUtf8(L"\u5FC3\u7406\u5B66"), "2026-03-16", 52.0, toUtf8(L"\u8BF4\u670D\u5FC3\u7406\u5B66\u7ECF\u5178")},
            {"The Wealth of Nations", "Economics", "2026-03-17", 72.0, "Classical economics"},
            {toUtf8(L"\u806A\u660E\u7684\u6295\u8D44\u8005"), toUtf8(L"\u7ECF\u6D4E"), "2026-03-18", 69.0, toUtf8(L"\u4EF7\u503C\u6295\u8D44\u539F\u5219")},
            {"To Kill a Mockingbird", "Literature", "2026-03-19", 49.0, "American classic novel"},
            {toUtf8(L"\u7EA2\u697C\u68A6"), toUtf8(L"\u6587\u5B66"), "2026-03-20", 58.0, toUtf8(L"\u4E2D\u56FD\u53E4\u5178\u540D\u8457")},
        };

        const int count = static_cast<int>(sizeof(demos) / sizeof(demos[0]));
        // 初始化数据按数组顺序写入，ID 从 1 递增。
        for (int i = 0; i < count; ++i) {
            // 逐行写入，便于人工查看和调试。
            ofs << (i + 1) << '|'
                << demos[i].title << '|'
                << demos[i].category << '|'
                << demos[i].date << '|'
                << demos[i].price << '|'
                << demos[i].intro << '\n';
        }
    }

    return true;
}

// 从文件加载到链表；失败返回 false。
bool Storage::loadFromFile(const std::string& filename, BookList& list) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        return false;
    }

    // 每次重载前先清空内存链表，避免重复叠加。
    list.clear();
    std::string line;
    while (std::getline(ifs, line)) {
        // 清理首尾空白，避免额外空格导致解析失败。
        line = StringUtil::trim(line);
        if (line.empty()) {
            continue;
        }
        Book b;
        // 异常行直接跳过，保证加载过程健壮。
        if (parseBookLine(line, b)) {
            list.addBook(b);
        }
    }
    return true;
}

// 将当前链表快照写回文件；失败返回 false。
bool Storage::saveToFile(const std::string& filename, const BookList& list) {
    // 每次保存都整体覆盖，课程规模下实现简单且行为可预期。
    std::ofstream ofs(filename, std::ios::trunc);
    if (!ofs.is_open()) {
        return false;
    }

    // 序列化时先导出顺序数组，再逐行写入。
    const std::vector<Book> arr = list.toVector();
    for (const Book& b : arr) {
        // 字段顺序需与 parseBookLine 完全一致。
        ofs << b.id << '|'
            << b.title << '|'
            << b.category << '|'
            << b.buyDate << '|'
            << b.price << '|'
            << b.intro << '\n';
    }
    return true;
}
