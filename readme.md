
---

## 📚 个人图书收藏与管理系统

> 数据结构课程设计 - 基于单链表的图形化图书管理

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![EasyX](https://img.shields.io/badge/EasyX-Graphics-green.svg)](https://easyx.cn/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

---

## ✨ 功能特性

| 功能 | 说明 |
|------|------|
| 📖 **图书管理** | 增删改查完整功能 |
| 🔍 **多字段搜索** | 支持书名/分类/日期/简介关键字检索 |
| 📊 **智能排序** | 按价格/日期/书名排序 |
| 💾 **数据持久化** | 自动保存到文件，程序启动自动加载 |
| 🎨 **图形界面** | EasyX 图形库，支持主题切换 |
| 📝 **输入验证** | 日期/价格格式自动校验 |

---

## 🖼️ 界面预览

![主界面](assets/screenshot.png)

*左側显示图书列表，右侧操作面板，底部日志区域*

---

## 🚀 快速开始

### 环境要求

- **操作系统**: Windows 10/11
- **编译器**: Visual Studio 2019 或更高版本
- **图形库**: [EasyX](https://easyx.cn/)

### 编译方法

1. **克隆仓库**
   ```bash
   git clone https://github.com/joes133/library-management-system.git
   ```

2. **打开项目**
   - 用 Visual Studio 打开 `d3.3.sln`

3. **安装 EasyX**（如果提示缺少库）
   - 访问 https://easyx.cn/
   - 下载并安装

4. **编译运行**
   - 按 `Ctrl+F5` 运行

---

## 📖 使用说明

### 基本操作

1. **添加图书**
   - 点击"添加图书"按钮
   - 输入书名、分类、购买日期、价格、简介
   - 点击确认保存

2. **查询图书**
   - 点击"查询图书"
   - 输入关键字（支持模糊搜索）
   - 查看匹配结果

3. **修改/删除**
   - 查询后直接编辑
   - 或输入 ID 进行修改/删除

4. **排序**
   - 点击"排序"按钮
   - 选择按价格/日期/书名排序

### 数据文件

- **位置**: `data/books.txt`
- **格式**: `id|title|category|buyDate|price|intro`
- **重置**: 删除 `books.txt` 后重启程序恢复默认数据

---

## 📁 项目结构

```
library-management-system/
├── d3.3.cpp              # 主程序入口
├── src/
│   ├── model/            # 数据模型
│   │   ├── Book.h        # 图书类
│   │   └── BookNode.h    # 链表节点
│   ├── core/             # 核心逻辑
│   │   ├── BookList.*    # 链表管理
│   │   ├── Storage.*     # 文件读写
│   │   └── Validator.*   # 输入验证
│   ├── ui/               # 界面绘制
│   │   ├── UI.*          # 主界面
│   │   ├── Pages.*       # 页面流程
│   │   └── Widgets.*     # UI 组件
│   └── utils/            # 工具函数
│       └── StringUtil.*  # 字符串处理
├── data/                 # 数据文件
│   ├── books.txt         # 图书数据库
│   └── theme.cfg         # 主题配置
├── assets/               # 素材资源
│   └── bg/               # 背景图片
└── README.md
```

---

## 🛠️ 技术栈

| 技术 | 用途 |
|------|------|
| **C++ 17** | 主要编程语言 |
| **EasyX** | 图形界面库 |
| **单链表** | 数据结构核心 |
| **文件 I/O** | 数据持久化 |

---

## 📋 核心功能实现

### 数据结构
```cpp
// 图书信息
struct Book {
    int id;
    std::string title;
    std::string category;
    std::string buyDate;
    double price;
    std::string intro;
};

// 链表节点
struct BookNode {
    Book data;
    BookNode* next;
};
```

### 时间复杂度
| 操作 | 复杂度 |
|------|--------|
| 添加 | O(1) |
| 删除 | O(n) |
| 查询 | O(n) |
| 排序 | O(n log n) |

---

## 🎨 主题支持

程序内置 5 套背景主题，可在主界面点击"切换主题"轮换。

主题配置保存在 `data/theme.cfg`，记录当前主题索引。

---

## 📝 默认数据

首次运行会自动生成 20 本演示图书（中英文混合），包含：
- 经典编程书籍
- 小说
- 教材

---

## 🔧 常见问题

### Q: 程序无法启动？
A: 检查是否安装了 EasyX 图形库。

### Q: 中文显示乱码？
A: 确保数据文件是 UTF-8 编码。

### Q: 如何恢复默认数据？
A: 删除 `data/books.txt` 后重新运行程序。

### Q: 编译报错？
A: 使用 Visual Studio 2019 或更高版本，确保 C++17 支持。

---

## 📄 License

MIT License - 可自由使用、修改、分发

---

## 👨‍💻 作者

**Joes133**  

---

## 📧 联系方式

- GitHub: [@joes133](https://github.com/joes133)
- Email: 3784428740@qq.com

---

## 🌟 Star History

如果这个项目对你有帮助，欢迎 ⭐ Star 支持！

---
## 😄 其他说明
** 如您使用本项目的UI设计，请自行更改位于文件UI.cpp下void drawMainUI函数中outtextxy模块的'c'版权说明 和 asserts/bg/images.的背景图资源 ** 😄
配置readme_dev用于详细了解技术架构与算法设计