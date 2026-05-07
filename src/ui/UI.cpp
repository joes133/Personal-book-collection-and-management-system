#include "UI.h"

#include <graphics.h>
#include <ctime>
#include <deque>
#include <fstream>
#include <io.h>
#include <string>
#include <vector>
#include <windows.h>

#include "../core/Storage.h"
#include "../model/Book.h"
#include "Pages.h"

/*
  UI.cpp
 -----------------------------------------
 主界面职责：
 1) 绘制：背景、标题、左侧书籍区、右侧按钮区、日志区
 2) 事件：处理鼠标点击并分发到对应业务流程
 3) 状态：维护当前主题、当前批次偏移、最近日志
 
 说明：
 主界面采用“分批展示”策略，每次显示 pageSize 条。
 点击 ListAll 按钮并不新开窗口，而是切换到下一批数据。
 */

namespace {
// 矩形按钮模型：同时用于命中检测和绘制。
struct Button {
    int x1;
    int y1;
    int x2;
    int y2;
    std::wstring label;
};

struct LogItem {
    std::wstring timeText;
    std::wstring action;
    std::wstring detail;
};

bool inButton(const Button& b, int x, int y) {
    // 基于矩形区域做点击命中判断。
    return x >= b.x1 && x <= b.x2 && y >= b.y1 && y <= b.y2;
}

std::wstring nowTimeText() {
    // 日志时间统一格式：HH:MM:SS。
    std::time_t t = std::time(nullptr);
    std::tm tmv{};
    localtime_s(&tmv, &t);
    wchar_t buf[16] = {0};
    wcsftime(buf, 16, L"%H:%M:%S", &tmv);
    return buf;
}

// 写入一条日志到队首。
void addLog(std::deque<LogItem>& logs, const std::wstring& action, const std::wstring& detail) {
    // 仅保留最近日志，避免挤占界面区域。
    logs.push_front(LogItem{nowTimeText(), action, detail});
    while (logs.size() > 5) {
        logs.pop_back();
    }
}

// UTF-8 字符串转宽字符，用于 EasyX 绘制。
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

// 超长文本裁剪，末尾追加省略号。
std::wstring clipText(const std::wstring& s, size_t maxLen) {
    if (s.size() <= maxLen) {
        return s;
    }
    if (maxLen <= 3) {
        return s.substr(0, maxLen);
    }
    return s.substr(0, maxLen - 3) + L"...";
}

// 绘制通用面板（卡片容器）。
void drawPanel(int x1, int y1, int x2, int y2, const wchar_t* title, bool fillPanel) {
    // 通用卡片绘制：边框 + 可选底色 + 标题。
    setlinecolor(RGB(205, 214, 228));
    if (fillPanel) {
        setfillcolor(RGB(248, 251, 255));
        solidrectangle(x1, y1, x2, y2);
    }
    rectangle(x1, y1, x2, y2);

    settextcolor(RGB(60, 70, 90));
    settextstyle(18, 0, L"Microsoft YaHei");
    outtextxy(x1 + 12, y1 + 10, title);
}

// 按钮底色映射。
COLORREF getButtonFillColor(const std::wstring& label) {
    // 按功能语义分色，提升操作辨识度。
    if (label == L"Add" || label == L"Edit") return RGB(222, 236, 255);
    if (label == L"Search" || label == L"Category" || label == L"ListAll") return RGB(220, 245, 245);
    if (label == L"Theme") return RGB(236, 226, 255);
    if (label == L"Delete") return RGB(255, 230, 225);
    if (label == L"Exit") return RGB(238, 233, 233);
    return RGB(232, 240, 255);
}

// 按钮边框色映射。
COLORREF getButtonLineColor(const std::wstring& label) {
    if (label == L"Add" || label == L"Edit") return RGB(72, 120, 210);
    if (label == L"Search" || label == L"Category" || label == L"ListAll") return RGB(60, 150, 150);
    if (label == L"Theme") return RGB(130, 95, 200);
    if (label == L"Delete") return RGB(210, 95, 70);
    if (label == L"Exit") return RGB(145, 120, 120);
    return RGB(75, 120, 210);
}

// 绘制单个按钮。
void drawButton(const Button& b) {
    // 按钮文字位置统一偏移，保证视觉对齐。
    setlinecolor(getButtonLineColor(b.label));
    setfillcolor(getButtonFillColor(b.label));
    solidrectangle(b.x1, b.y1, b.x2, b.y2);
    rectangle(b.x1, b.y1, b.x2, b.y2);

    settextcolor(RGB(40, 50, 70));
    settextstyle(20, 0, L"Microsoft YaHei");
    outtextxy(b.x1 + 16, b.y1 + 8, b.label.c_str());
}

// 绘制左侧书籍列表（分批展示）。
void drawBookPreview(const BookList& list, int listOffset, int pageSize) {
    // 主界面只展示一批数据，ListAll 用于切换到下一批。
    const std::vector<Book> books = list.toVector();
    const int panelX = 20;
    const int panelY = 80;
    const int panelW = 700;
    const int startY = panelY + 46;
    const int rowH = 28;

    settextstyle(16, 0, L"Consolas");
    settextcolor(RGB(35, 45, 65));
    outtextxy(panelX + 14, startY, L"ID");
    outtextxy(panelX + 70, startY, L"Title");
    outtextxy(panelX + 318, startY, L"Category");
    outtextxy(panelX + 486, startY, L"Date");
    outtextxy(panelX + 622, startY, L"Price");
    line(panelX + 12, startY + 24, panelW - 12, startY + 24);

    // total 为总记录数，begin/end 为当前批次区间。
    // 当 total=0 时，begin/end 归零，避免取模和越界。
    const int total = static_cast<int>(books.size());
    // begin/end 定义当前批次的左右边界。
    const int begin = (total == 0) ? 0 : (listOffset % total);
    const int end = (total == 0) ? 0 : (begin + pageSize > total ? total : begin + pageSize);
    std::wstring info = L"Showing " + std::to_wstring(begin + (total == 0 ? 0 : 1)) + L"-" +
                        std::to_wstring(end) + L" / " + std::to_wstring(total);
    outtextxy(panelX + 472, startY - 28, info.c_str());

    int y = startY + 32;
    for (int i = begin; i < end; ++i) {
        const Book& b = books[static_cast<size_t>(i)];
        if (((i - begin) % 2) == 1) {
            setfillcolor(RGB(248, 250, 253));
            solidrectangle(panelX + 8, y - 3, panelW - 8, y + rowH - 3);
        }
        settextcolor(RGB(35, 45, 65));
        outtextxy(panelX + 14, y, std::to_wstring(b.id).c_str());
        outtextxy(panelX + 70, y, clipText(toWide(b.title), 22).c_str());
        outtextxy(panelX + 318, y, clipText(toWide(b.category), 14).c_str());
        outtextxy(panelX + 486, y, clipText(toWide(b.buyDate), 10).c_str());
        // 价格统一保留两位小数，避免列宽抖动。
        wchar_t priceBuf[32] = {0};
        swprintf_s(priceBuf, L"%.2f", b.price);
        outtextxy(panelX + 622, y, clipText(priceBuf, 7).c_str());
        y += rowH;
    }
}

// 检查主题图文件是否存在。
bool fileExistsW(const std::wstring& path) {
    return _waccess(path.c_str(), 0) == 0;
}

// 读取上次主题索引；非法值回退 0。
int loadSavedThemeIndex(const std::string& cfgFile, int themeCount) {
    std::ifstream ifs(cfgFile);
    if (!ifs.is_open()) {
        return 0;
    }
    int idx = 0;
    ifs >> idx;
    if (idx < 0 || idx >= themeCount) {
        return 0;
    }
    return idx;
}

// 保存当前主题索引。
void saveThemeIndex(const std::string& cfgFile, int idx) {
    std::ofstream ofs(cfgFile, std::ios::trunc);
    if (!ofs.is_open()) {
        return;
    }
    ofs << idx;
}

// 绘制背景图。
void drawBackground(const std::vector<std::wstring>& bgPaths, int bgIndex, IMAGE& bgImage) {
    // 主题图缺失时回退到纯色背景。
    if (bgIndex >= 0 && bgIndex < static_cast<int>(bgPaths.size()) && fileExistsW(bgPaths[bgIndex])) {
        loadimage(&bgImage, bgPaths[bgIndex].c_str(), 1000, 680, true);
        putimage(0, 0, &bgImage);
        return;
    }
    setbkcolor(RGB(245, 247, 250));
    cleardevice();
}

// 绘制右下角日志面板。
void drawLogPanel(const std::deque<LogItem>& logs) {
    // 日志区只展示最近 5 条，防止右侧信息拥挤。
    const int x1 = 720;
    const int y1 = 450;
    int y = y1 + 44;
    settextstyle(15, 0, L"Consolas");
    settextcolor(RGB(55, 60, 75));

    if (logs.empty()) {
        outtextxy(x1 + 14, y, L"No logs yet.");
        return;
    }

    for (const LogItem& item : logs) {
        std::wstring line = L"[" + item.timeText + L"] " + item.action + L" - " + item.detail;
        if (line.size() > 34) {
            line = line.substr(0, 34) + L"...";
        }
        outtextxy(x1 + 12, y, line.c_str());
        y += 30;
    }
}

// 组合绘制整页主界面。
void drawMainUI(
    const BookList& list,
    const std::vector<Button>& buttons,
    const std::vector<std::wstring>& bgPaths,
    int bgIndex,
    IMAGE& bgImage,
    const std::deque<LogItem>& logs,
    int listOffset,
    int pageSize) {
    cleardevice();
    // 绘制顺序：背景 -> 顶栏 -> 面板 -> 按钮 -> 列表 -> 日志。
    drawBackground(bgPaths, bgIndex, bgImage);

    setlinecolor(RGB(210, 215, 225));
    rectangle(16, 14, 930, 62);
    settextstyle(28, 0, L"Microsoft YaHei");
    settextcolor(RGB(28, 36, 56));
    outtextxy(28, 22, L"\u6B22\u8FCE\u4F7F\u7528\u56FE\u4E66\u7BA1\u7406\u7CFB\u7EDF / Welcome to Book Management System");
    settextstyle(18, 0, L"Microsoft YaHei");
    outtextxy(744, 38, L"\u00A9 2026 by JiaoXin");

    drawPanel(20, 80, 700, 640, L"Books", false);
    drawPanel(730, 80, 970, 430, L"Actions", true);
    drawPanel(730, 450, 970, 640, L"Recent Logs", true);

    for (const Button& b : buttons) {
        drawButton(b);
    }
    drawBookPreview(list, listOffset, pageSize);
    drawLogPanel(logs);
}
}  // namespace

// 主界面循环：处理按钮点击并驱动数据操作。
void UI::runMainLoop(BookList& list) {
    // 事件驱动 + 批量绘制，减少闪烁并提升稳定性。
    initgraph(1000, 680);
    setbkcolor(WHITE);
    cleardevice();
    BeginBatchDraw();

    // 右侧按钮布局按“增删改查分类/列表/主题”顺序排列。
    std::vector<Button> buttons = {
        {744, 80, 956, 118, L"Add"},
        {744, 128, 956, 166, L"Delete"},
        {744, 176, 956, 214, L"Edit"},
        {744, 224, 956, 262, L"Search"},
        {744, 272, 956, 310, L"Category"},
        {744, 320, 956, 358, L"ListAll"},
        {744, 368, 956, 406, L"Theme"},
        {744, 590, 956, 628, L"Exit"},
    };
    // 图书数据文件。
    const std::string dbFile = "data/books.txt";
    // 主题索引配置文件。
    const std::string themeCfgFile = "data/theme.cfg";
    std::vector<std::wstring> bgPaths = {
        L"assets/bg/bg1.jpg",
        L"assets/bg/bg2.jpg",
        L"assets/bg/bg3.jpg",
        L"assets/bg/bg4.jpg",
        L"assets/bg/bg5.jpg",
    };
    int bgIndex = loadSavedThemeIndex(themeCfgFile, static_cast<int>(bgPaths.size()));
    IMAGE bgImage;
    std::deque<LogItem> logs;
    addLog(logs, L"SYSTEM", L"UI initialized");
    int listOffset = 0;
    const int pageSize = 10;

    bool running = true;      // 主循环开关
    bool needRedraw = true;   // 脏标记：状态变化后再重绘
    while (running) {
        if (needRedraw) {
            drawMainUI(list, buttons, bgPaths, bgIndex, bgImage, logs, listOffset, pageSize);
            FlushBatchDraw();
            needRedraw = false;
        }

        // 阻塞等待鼠标消息，避免空转占用 CPU。
        ExMessage msg;
        getmessage(&msg, EX_MOUSE);
        if (msg.message == WM_LBUTTONDOWN) {
            // 各按钮操作后通常需要：
            // 1) 执行业务逻辑
            // 2) 需要时保存文件
            // 3) 写日志并触发重绘
            if (inButton(buttons[0], msg.x, msg.y)) {
                Pages::addBookFlow(list);
                const bool ok = Storage::saveToFile(dbFile, list);
                addLog(logs, L"ADD", ok ? L"saved" : L"save failed");
                listOffset = 0;
                needRedraw = true;
            } else if (inButton(buttons[1], msg.x, msg.y)) {
                Pages::deleteBookFlow(list);
                const bool ok = Storage::saveToFile(dbFile, list);
                addLog(logs, L"DELETE", ok ? L"saved" : L"save failed");
                listOffset = 0;
                needRedraw = true;
            } else if (inButton(buttons[2], msg.x, msg.y)) {
                Pages::editBookFlow(list);
                const bool ok = Storage::saveToFile(dbFile, list);
                addLog(logs, L"EDIT", ok ? L"saved" : L"save failed");
                listOffset = 0;
                needRedraw = true;
            } else if (inButton(buttons[3], msg.x, msg.y)) {
                Pages::searchBookFlow(list);
                addLog(logs, L"SEARCH", L"done");
                needRedraw = true;
            } else if (inButton(buttons[4], msg.x, msg.y)) {
                Pages::sortFlow(list);
                const bool ok = Storage::saveToFile(dbFile, list);
                addLog(logs, L"CATEGORY", ok ? L"saved" : L"save failed");
                listOffset = 0;
                needRedraw = true;
            } else if (inButton(buttons[5], msg.x, msg.y)) {
                // 在主界面原地翻批次，不额外打开新页面。
                const int total = static_cast<int>(list.toVector().size());
                if (total > 0) {
                    listOffset += pageSize;
                    if (listOffset >= total) {
                        listOffset = 0;
                    }
                }
                addLog(logs, L"LISTALL", L"next batch");
                needRedraw = true;
            } else if (inButton(buttons[6], msg.x, msg.y)) {
                bgIndex = (bgIndex + 1) % static_cast<int>(bgPaths.size());
                saveThemeIndex(themeCfgFile, bgIndex);
                addLog(logs, L"THEME", L"changed");
                needRedraw = true;
            } else if (inButton(buttons[7], msg.x, msg.y)) {
                addLog(logs, L"SYSTEM", L"exit");
                running = false;
            }
        }
    }

    EndBatchDraw();
    closegraph();
}
