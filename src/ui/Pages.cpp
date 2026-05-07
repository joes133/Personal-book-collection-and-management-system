#include "Pages.h"

#include <graphics.h>
#include <windows.h>
#include <string>
#include <vector>

#include "../core/Validator.h"
#include "../model/Book.h"

/*
 Pages.cpp
 -----------------------------------------
 该模块提供“业务交互流程”：
 新增、删除、修改、查询、排序
 通过 InputBox/MessageBox 与用户交互
 
 设计原则：
 输入采集尽量统一（askText）
 校验逻辑集中（Validator）
 相同编辑流程抽成公共函数（editById）
 */

namespace {
// EasyX 输入输出需要宽字符，这里统一做 UTF-8 与宽字符互转。
std::wstring toWide(const std::string& s) {
    if (s.empty()) {
        return L"";
    }
    int wlen = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (wlen <= 0) {
        return L"";
    }
    std::wstring ws(static_cast<size_t>(wlen), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], wlen);
    if (!ws.empty() && ws.back() == L'\0') {
        ws.pop_back();
    }
    return ws;
}

std::string toUtf8(const std::wstring& ws) {
    if (ws.empty()) {
        return "";
    }
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) {
        return "";
    }
    std::string s(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &s[0], len, nullptr, nullptr);
    if (!s.empty() && s.back() == '\0') {
        s.pop_back();
    }
    return s;
}

std::string askText(const std::wstring& prompt, const std::wstring& title, const std::string& defaultVal = "") {
    // 统一输入入口：支持默认值回填，减少重复弹窗代码。
    wchar_t buf[512] = {0};
    std::wstring def = toWide(defaultVal);
    if (!def.empty()) {
        wcsncpy_s(buf, def.c_str(), _TRUNCATE);
    }
    InputBox(buf, 512, prompt.c_str(), title.c_str(), def.c_str());
    return toUtf8(buf);
}

void showInfo(const std::wstring& text, const std::wstring& title = L"Info") {
    // 统一消息提示入口，便于后续替换为状态栏提示。
    MessageBoxW(GetHWnd(), text.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
}

// 格式化单行书籍信息。
std::wstring formatBookLine(const Book& b) {
    std::wstring line = std::to_wstring(b.id) + L" | " + toWide(b.title) + L" | " +
                        toWide(b.category) + L" | " + toWide(b.buyDate) + L" | " +
                        toWide(std::to_string(b.price));
    return line;
}

void showBookList(const std::vector<Book>& books, const std::wstring& title) {
    // 简版列表弹窗：主要用于搜索结果快速预览。
    if (books.empty()) {
        showInfo(L"No records.", title);
        return;
    }

    std::wstring text = L"ID | Title | Category | Date | Price\n";
    text += L"-----------------------------------------------\n";
    for (size_t i = 0; i < books.size() && i < 20; ++i) {
        text += formatBookLine(books[i]) + L"\n";
    }
    if (books.size() > 20) {
        text += L"... (more records hidden)\n";
    }
    showInfo(text, title);
}

bool editById(BookList& list, int id, const std::wstring& dialogTitle, const std::wstring& okText) {
    // 公共编辑流程：直接修改、查询后修改都复用这里。
    Book* oldBook = list.findById(id);
    if (oldBook == nullptr) {
        showInfo(L"Book not found.", dialogTitle);
        return false;
    }

    // 先拷贝旧记录，再按字段覆盖，避免只改部分字段时丢数据。
    Book edited = *oldBook;
    // 输入时回填旧值，便于只改少数字段。
    edited.title = askText(L"Input title", dialogTitle, oldBook->title);
    edited.category = askText(L"Input category", dialogTitle, oldBook->category);
    edited.buyDate = askText(L"Input buy date (YYYY-MM-DD)", dialogTitle, oldBook->buyDate);
    std::string priceText = askText(L"Input price", dialogTitle, std::to_string(oldBook->price));
    edited.intro = askText(L"Input intro", dialogTitle, oldBook->intro);

    if (!Validator::isNonEmpty(edited.title) || !Validator::isNonEmpty(edited.category) ||
        !Validator::isValidDate(edited.buyDate) || !Validator::isValidPrice(priceText)) {
        showInfo(L"Invalid input, edit failed.", dialogTitle);
        return false;
    }

    edited.price = std::stod(priceText);
    edited.id = id;
    // 写回时强制保留原 ID，避免编辑过程中误改主键。
    list.updateById(id, edited);
    showInfo(okText, dialogTitle);
    return true;
}
}  // namespace

// 新增图书流程。
void Pages::addBookFlow(BookList& list) {
    Book b;
    // 新增记录的 ID 由链表当前最大 ID 自动生成。
    b.id = list.nextId();

    b.title = askText(L"Input title", L"Add Book");
    b.category = askText(L"Input category", L"Add Book");
    b.buyDate = askText(L"Input buy date (YYYY-MM-DD)", L"Add Book");
    std::string priceText = askText(L"Input price", L"Add Book");
    b.intro = askText(L"Input intro", L"Add Book");

    // 校验集中处理，避免界面层分散判断。
    if (!Validator::isNonEmpty(b.title) || !Validator::isNonEmpty(b.category) ||
        !Validator::isValidDate(b.buyDate) || !Validator::isValidPrice(priceText)) {
        showInfo(L"Invalid input, add failed.", L"Add Book");
        return;
    }

    b.price = std::stod(priceText);
    list.addBook(b);
    // 新增成功后给出新 ID，便于后续编辑/删除定位。
    showInfo(L"Added book. ID = " + std::to_wstring(b.id), L"Add Book");
}

// 按 ID 修改图书流程。
void Pages::editBookFlow(BookList& list) {
    // 编辑入口先拿 ID，再走公共 editById。
    std::string idText = askText(L"Input book id to edit", L"Edit Book");
    if (!Validator::isValidPrice(idText)) {
        showInfo(L"Invalid id.", L"Edit Book");
        return;
    }

    const int id = std::stoi(idText);
    editById(list, id, L"Edit Book", L"Updated.");
}

// 按 ID 删除图书流程。
void Pages::deleteBookFlow(BookList& list) {
    // 删除操作使用 ID 定位，避免书名重名导致误删。
    std::string idText = askText(L"Input book id to delete", L"Delete Book");
    if (!Validator::isValidPrice(idText)) {
        showInfo(L"Invalid id.", L"Delete Book");
        return;
    }
    int id = std::stoi(idText);
    // 删除结果通过返回值判断，避免二次查找。
    if (list.deleteById(id)) {
        showInfo(L"Deleted.", L"Delete Book");
    } else {
        showInfo(L"Not found.", L"Delete Book");
    }
}

// 查询图书流程；支持查询后直接修改。
void Pages::searchBookFlow(BookList& list) {
    // 关键字匹配 title/category/date/intro 四个字段。
    std::string key = askText(L"Input keyword", L"Search Book");
    std::vector<Book> found = list.findByKeyword(key);
    showBookList(found, L"Search Result");

    if (found.empty()) {
        return;
    }

    // 查询后可直接进入编辑，符合课程需求“查到后可修改”。
    std::string editChoice = askText(L"Edit after search? input y/n", L"Search Book", "n");
    if (!(editChoice == "y" || editChoice == "Y")) {
        return;
    }

    std::string idText = askText(L"Input book id to edit", L"Search Book");
    if (!Validator::isValidPrice(idText)) {
        showInfo(L"Invalid id.", L"Search Book");
        return;
    }

    int id = std::stoi(idText);
    // 仅允许修改当前查询结果里的记录，避免误操作。
    bool inResult = false;
    for (const Book& b : found) {
        if (b.id == id) {
            inResult = true;
            break;
        }
    }
    if (!inResult) {
        showInfo(L"ID is not in search results.", L"Search Book");
        return;
    }

    editById(list, id, L"Edit Book", L"Updated from search result.");
}

// 排序流程（当前挂接在 Category 按钮）。
void Pages::sortFlow(BookList& list) {
    // 当前“Category”按钮映射为排序入口。
    // 这里保留文字输入，是为了减少界面控件复杂度。
    std::string key = askText(L"Category key (sort by): price/date/title", L"Category");
    std::string order = askText(L"Order: asc/desc", L"Category", "asc");

    bool asc = !(order == "desc" || order == "DESC" || order == "Desc");
    // 支持 price/date/title 三个排序键。
    if (key == "price" || key == "PRICE" || key == "Price") {
        list.sortByPrice(asc);
    } else if (key == "date" || key == "DATE" || key == "Date") {
        list.sortByDate(asc);
    } else if (key == "title" || key == "TITLE" || key == "Title") {
        list.sortByTitle(asc);
    } else {
        showInfo(L"Unknown key.", L"Category");
        return;
    }
    showInfo(L"Sorted.", L"Category");
}
