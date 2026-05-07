#include "BookList.h"

#include <algorithm>

/*
 BookList.cpp
 -----------------------------------------
 该模块负责“内存中的图书链表管理”：
 1) 增删改查（按 ID 精确定位）
 2) 关键字检索（title/category/date/intro）
 3) 排序（转 vector 后排序再回写链表）
 
 说明：
 题目要求使用链表存储，因此核心结构保持单链表。
 为了降低排序复杂度，排序阶段采用“链表 <-> vector”转换。
 */

namespace {
// 关键字查询使用的基础子串匹配。
bool contains(const std::string& text, const std::string& key) {
    return text.find(key) != std::string::npos;
}
}  // namespace

// 初始化空链表。
BookList::BookList() : head_(nullptr) {}

// 析构时释放所有节点。
BookList::~BookList() {
    clear();
}

// 清空链表并释放内存。
void BookList::clear() {
    // 释放全部节点，避免重建链表或退出时内存泄漏。
    BookNode* cur = head_;
    while (cur != nullptr) {
        BookNode* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    head_ = nullptr;
}

// 判断链表是否为空。
bool BookList::isEmpty() const {
    return head_ == nullptr;
}

// 新增图书：采用尾插法。
void BookList::addBook(const Book& b) {
    // 采用尾插，保持记录插入顺序稳定。
    BookNode* node = new BookNode(b);
    if (head_ == nullptr) {
        head_ = node;
        return;
    }
    BookNode* cur = head_;
    while (cur->next != nullptr) {
        cur = cur->next;
    }
    cur->next = node;
}

// 按 ID 删除图书，删除成功返回 true。
bool BookList::deleteById(int id) {
    // 前驱+当前指针单趟删除。
    BookNode* prev = nullptr;
    BookNode* cur = head_;
    while (cur != nullptr) {
        if (cur->data.id == id) {
            if (prev == nullptr) {
                head_ = cur->next;
            } else {
                prev->next = cur->next;
            }
            delete cur;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

// 按 ID 更新整条记录，未找到返回 false。
bool BookList::updateById(int id, const Book& newBook) {
    // 先定位，再整条覆盖；字段层校验在上层 UI/Validator 中做。
    Book* target = findById(id);
    if (target == nullptr) {
        return false;
    }
    *target = newBook;
    return true;
}

// 按 ID 查找，返回可修改指针；找不到返回 nullptr。
Book* BookList::findById(int id) {
    // 链表顺序扫描：时间复杂度 O(n)。
    BookNode* cur = head_;
    while (cur != nullptr) {
        if (cur->data.id == id) {
            return &cur->data;
        }
        cur = cur->next;
    }
    return nullptr;
}

// 关键字查询：title/category/date/intro 任意命中即返回。
std::vector<Book> BookList::findByKeyword(const std::string& key) const {
    // 在可见字段中做匹配，intro 也参与“其他信息”检索。
    std::vector<Book> result;
    BookNode* cur = head_;
    while (cur != nullptr) {
        const Book& b = cur->data;
        if (contains(b.title, key) || contains(b.category, key) ||
            contains(b.buyDate, key) || contains(b.intro, key)) {
            result.push_back(b);
        }
        cur = cur->next;
    }
    return result;
}

// 按价格排序；asc=true 升序，false 降序。
void BookList::sortByPrice(bool asc) {
    // 工程折中：链表转 vector 后排序，逻辑更简洁且复杂度 O(n log n)。
    std::vector<Book> arr = toVector();
    std::sort(arr.begin(), arr.end(), [asc](const Book& a, const Book& b) {
        return asc ? a.price < b.price : a.price > b.price;
    });
    rebuildFromVector(arr);
}

// 按日期排序；日期格式使用 YYYY-MM-DD。
void BookList::sortByDate(bool asc) {
    // 日期采用 YYYY-MM-DD，字符串字典序等价于时间序。
    std::vector<Book> arr = toVector();
    std::sort(arr.begin(), arr.end(), [asc](const Book& a, const Book& b) {
        return asc ? a.buyDate < b.buyDate : a.buyDate > b.buyDate;
    });
    rebuildFromVector(arr);
}

// 按书名排序。
void BookList::sortByTitle(bool asc) {
    // 书名按字符串字典序排序。
    std::vector<Book> arr = toVector();
    std::sort(arr.begin(), arr.end(), [asc](const Book& a, const Book& b) {
        return asc ? a.title < b.title : a.title > b.title;
    });
    rebuildFromVector(arr);
}

// 导出为顺序数组，便于排序和序列化。
std::vector<Book> BookList::toVector() const {
    // 顺序保持与链表一致，便于 UI 分页显示。
    std::vector<Book> arr;
    BookNode* cur = head_;
    while (cur != nullptr) {
        arr.push_back(cur->data);
        cur = cur->next;
    }
    return arr;
}

// 由顺序数组重建链表。
void BookList::rebuildFromVector(const std::vector<Book>& arr) {
    // 排序后按结果重建链表，保持核心存储结构不变。
    clear();
    for (const Book& b : arr) {
        addBook(b);
    }
}

// 生成下一个可用 ID。
int BookList::nextId() const {
    // ID 规则：当前最大值 + 1，保证重启后仍唯一。
    // 该策略无需额外索引文件，课程设计场景足够稳定。
    int maxId = 0;
    BookNode* cur = head_;
    while (cur != nullptr) {
        if (cur->data.id > maxId) {
            maxId = cur->data.id;
        }
        cur = cur->next;
    }
    return maxId + 1;
}
