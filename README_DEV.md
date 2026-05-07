# 图书管理系统（开发说明版）

## 1. 项目目标与需求拆解

本项目是“数据结构课程设计”的图形化实现，核心目标：

1. 使用**单链表**管理图书数据  
2. 提供完整增删改查流程  
3. 支持多字段排序与关键字检索  
4. 通过文件读写保证数据持久化  
5. 提供可交互 UI（EasyX）

需求关键词对应关系：

- 线性表 -> `BookList` 单链表
- 查找 -> `findById` / `findByKeyword`
- 排序 -> `sortByPrice` / `sortByDate` / `sortByTitle`
- 文件 -> `Storage::loadFromFile` / `saveToFile`
- 交互 -> `UI::runMainLoop` + `Pages::*Flow`

---

## 2. 总体架构设计

采用分层组织，降低耦合：

- `model`：纯数据结构定义
- `core`：链表管理、持久化、输入校验
- `ui`：界面绘制与交互流程
- `utils`：通用字符串处理

数据流：

1. 程序启动 -> `Storage` 从文件读入 -> `BookList`
2. 用户操作 -> `Pages` 修改 `BookList`
3. 保存动作 -> `Storage` 写回文件
4. 退出 -> 再保存一次

---

## 3. 目录结构

```text
d3.3/
├─ d3.3.cpp
├─ README.md
├─ README_DEV.md
├─ src/
│  ├─ model/
│  │  ├─ Book.h
│  │  └─ BookNode.h
│  ├─ core/
│  │  ├─ BookList.h / BookList.cpp
│  │  ├─ Storage.h / Storage.cpp
│  │  └─ Validator.h / Validator.cpp
│  ├─ ui/
│  │  ├─ UI.h / UI.cpp
│  │  ├─ Pages.h / Pages.cpp
│  │  └─ Widgets.h / Widgets.cpp
│  └─ utils/
│     └─ StringUtil.h / StringUtil.cpp
├─ data/
│  ├─ books.txt
│  └─ theme.cfg
└─ assets/bg/
   ├─ bg1.jpg ... bg5.jpg
```

---

## 4. 核心数据结构

### 4.1 `Book`

字段：

- `id`：唯一主键（整型）
- `title`：书名
- `category`：分类
- `buyDate`：购买日期（`YYYY-MM-DD`）
- `price`：价格（`double`）
- `intro`：其他信息/简介

### 4.2 `BookNode`

单链表节点：

- `Book data`
- `BookNode* next`

### 4.3 `BookList`

职责：

- 增删改查
- 关键字查询
- 排序
- 导出/重建
- 生成下一个 ID

设计说明：

- 仍以链表作为主存储，满足题目要求
- 排序时临时转 `vector`，降低实现复杂度

---

## 5. 模块详细说明

## 5.1 `core/BookList.cpp`

关键函数：

- `addBook(const Book&)`：尾插，保持数据录入顺序
- `deleteById(int)`：单趟删除
- `updateById(int, const Book&)`：整条记录覆盖
- `findByKeyword(const std::string&)`：在 `title/category/date/intro` 上匹配
- `sortBy*`：转 `vector` + `std::sort` + 重建链表
- `nextId()`：`max(id)+1`

复杂度：

- 查找/删除/更新：`O(n)`
- 排序：`O(n log n)`

## 5.2 `core/Storage.cpp`

职责：

- `initDatabaseIfNeeded`：不存在则建库并写演示数据
- `loadFromFile`：逐行解析，构建链表
- `saveToFile`：覆盖写入完整快照

文件格式：

`id|title|category|buyDate|price|intro`

健壮性策略：

- 非法行跳过，不阻断加载
- 保存采用全量覆盖，逻辑简单可预期

## 5.3 `core/Validator.cpp`

提供输入校验：

- 日期格式校验（`YYYY-MM-DD`）
- 价格格式校验（非负数字）
- 非空校验

## 5.4 `ui/UI.cpp`

职责：

- 绘制主界面（背景、标题、左列表、右操作、日志）
- 按钮命中检测与事件分发
- 分批显示（`ListAll` 切下一批）
- 主题切换与主题索引保存

界面布局：

- 左侧：图书列表（每批 10 条）
- 右侧：操作按钮
- 右下：最近日志（5 条）

## 5.5 `ui/Pages.cpp`

职责：

- `addBookFlow`：新增流程
- `editBookFlow`：按 ID 修改
- `deleteBookFlow`：按 ID 删除
- `searchBookFlow`：关键字查询 + 查询后可编辑
- `sortFlow`：排序交互入口

复用设计：

- 公共编辑逻辑抽到 `editById(...)`，减少重复代码

---

## 6. 编码与跨平台说明

当前策略：

- 文件存储统一 UTF-8
- EasyX 界面使用宽字符接口
- 中文演示数据通过宽字符转 UTF-8 再写盘

这样做的原因：

- 避免源码文件编码差异导致编译错误
- 保证文件在不同编辑器中可读性更稳定

---

## 7. 交互流程（主流程）

1. 启动程序
2. 初始化数据库文件（首次）
3. 加载数据到链表
4. 进入 `UI::runMainLoop`
5. 用户点击按钮触发对应 Flow
6. 增删改排后自动保存
7. 退出前再次保存并关闭窗口

---

## 8. 默认数据与重置方式

- 默认会生成 20 本演示图书（中英文混合）
- 仅在 `data/books.txt` 不存在时写入

若需要恢复默认数据：

1. 删除 `data/books.txt`
2. 重新运行程序

---

## 9. 已实现亮点

- 图形界面结构清晰（左数据右操作下日志）
- 分批浏览机制（避免列表超界）
- 主题轮换 + 配置记忆
- 查询后可直接修改
- 中文/英文混合数据支持

---

## 10. 可继续优化方向

- 把多次弹窗输入改为单弹窗表单
- 增加“重置默认数据”按钮
- 增加分页控件（上一页/下一页）
- 将编码转换函数抽到 `utils` 统一复用
- 导出 CSV 与导入校验
