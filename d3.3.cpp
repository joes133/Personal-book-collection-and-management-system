#include <iostream>
#include <string>

#include "src/core/BookList.h"
#include "src/core/Storage.h"
#include "src/ui/UI.h"

/*
 程序入口
 -----------------------------------------
 启动顺序：
 1) 初始化/检查数据库文件
 2) 将磁盘数据加载到内存链表
 3) 进入图形界面主循环
 4) 退出前保存当前数据
 */

int main() {
    BookList books;
    // 数据文件放在项目目录下，便于演示和重置。
    const std::string dbFile = "data/books.txt";

    // 首次运行时创建数据库文件，并写入演示数据。
    if (!Storage::initDatabaseIfNeeded(dbFile, true)) {
        std::cerr << "Failed to initialize database file.\n";
        return 1;
    }

    // 进入界面循环前，先把磁盘数据加载到内存链表。
    if (!Storage::loadFromFile(dbFile, books)) {
        std::cerr << "Failed to load data from file.\n";
        return 1;
    }

    // UI阶段主要操作内存数据；正常退出前再落盘一次。
    UI::runMainLoop(books);

    if (!Storage::saveToFile(dbFile, books)) {
        std::cerr << "Failed to save data to file.\n";
        return 1;
    }

    return 0;
}
