# log.md：项目问题与修改记录

本文件用于记录项目开发和运行过程中出现的问题、重要修改、当前疑问和后续处理结论。记录应保持简洁清晰，不需要逐行描述代码改动，但要能让后续开发者快速知道：发生了什么、影响哪里、如何处理、当前是否解决。

---

## 记录格式

```text
日期 / 阶段：
类型：问题 / 修改 / 决策 / 待确认
概述：
影响范围：
处理方式：
当前状态：未处理 / 处理中 / 已解决 / 暂缓
后续注意：
```

---

## 记录条目

### 2026-05-11 / 文档规划阶段

类型：修改

概述：
完善 `solution.md`，补充节点式图像处理工作流工具的目标、GUI 操作流程、节点类型、分支汇合、端口规则、执行验证和防崩溃要求。

影响范围：
`solution.md`

处理方式：
在原章节中融合补充内容，避免把新要求单独堆在文末。

当前状态：
已解决

后续注意：
后续实现代码时，应优先遵守 `solution.md`、`struct.md`、`agent.md` 中的结构和工程规则。

---

### 2026-05-11 / 文档规划阶段

类型：决策

概述：
项目采用 C++17 + Qt 6 + CMake，基础功能使用 Qt 原生 `QImage`、`QPainter`、`QGraphicsView` 等能力，不把 OpenCV 等外部库作为基础依赖。

影响范围：
整体架构、CMake、节点实现、跨平台构建

处理方式：
外部库只能作为可选后端；基础节点必须不依赖外部库也能运行。

当前状态：
已解决

后续注意：
如果新增外部库，需要在 CMake 中做可选检测，并在 UI 中隐藏或禁用不可用节点。

---

### 2026-05-11 / 基础实现阶段

类型：修改

概述：
按 `solution.md` 和 `struct.md` 建立 Qt 6 Widgets + CMake 项目，实现节点工厂、基础图像处理节点、workflow JSON 保存加载、DAG 验证、拓扑执行、GUI 画布/属性/预览/日志，以及 `--no-gui --workflow` 命令行执行模式。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/` 源码目录、`resources/samples/`、`resources/workflows/`、`.gitignore`

处理方式：
基础节点通过 `ImageNode` 多态和 `NodeFactory` 注册，图像算法使用 Qt 原生 `QImage`/`QPainter`，GUI 和命令行共用 `workflow/`、`nodes/`、`processing/`。

当前状态：
已解决

后续注意：
当前 GUI 连线采用先点输出端口、再点输入端口的基础交互；复杂拖拽连线、节点执行状态着色和更细粒度测试可后续增强。

---

### 2026-05-11 / GUI 交互增强阶段

类型：修改

概述：
为节点画布增加节点右键菜单，支持复制执行节点、删除执行节点、修改参数；增加画布右下角加减缩放按钮和鼠标滚轮缩放；双击节点列表添加节点时会自动避让已有节点位置，避免重合。

影响范围：
`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
右键菜单在 `WorkflowScene` 中识别节点并委托 `MainWindow` 操作；复制节点复用 `NodeFactory` 创建新节点并通过 `saveParams/loadParams` 复制参数；缩放通过自定义 `QGraphicsView` 处理滚轮和按钮；新增节点位置使用矩形碰撞检测逐步偏移。

当前状态：
已解决

后续注意：
当前复制节点只复制节点本身和参数，不复制与原节点相关的连线。

---

### 2026-05-11 / 题目要求补齐阶段

类型：修改

概述：
对照 `Problem.html` 补齐单独删除连线功能，并补充题目完成度、AI 工具使用说明、README 运行路径和 Visual Studio 提交建议。

影响范围：
`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`solution.md`、`README.md`

处理方式：
将画布连线替换为可选中的 `EdgeItem`，保存 edge index；支持选中连线后按 Delete/Backspace 删除，也支持右键连线菜单删除。文档中列出已完成项、仍需用户准备的提交材料和 Windows 生成 VS 工程方式。

当前状态：
已解决

后续注意：
Visual Studio 工程文件建议在 Windows + Qt 环境中由 CMake 生成，不在 macOS 下手写未验证的 `.sln/.vcxproj`。

---

### 2026-05-11 / 文档修正阶段

类型：修改

概述：
调整项目结构说明：`main.cpp` 放入与项目同名的 `ImageNodeEditor/` 子目录；`resources.qrc` 改为可选资源清单；补充外部图片导入、复制到 workflow 同级 `assets/` 目录复用、GUI 连线与 `WorkflowGraph::Edge` 数据模型关系、函数签名概念说明。

影响范围：
`solution.md`、`struct.md`、`agent.md`

处理方式：
更新目录结构示例、CMake 示例路径、资源文件说明、`ImageInputNode` 需求、连线模型说明和辅助编码工具排错说明。

当前状态：
已解决

后续注意：
实现工程时应按 `ImageNodeEditor/main.cpp` 和同名源码目录组织文件；基础版本可以不创建 `resources.qrc`，但必须支持用户通过 `QFileDialog` 导入外部图片。

---

### 2026-05-12 / GUI 交互增强阶段

类型：修改

概述：
增加预览图片点击弹窗、画布背景长按拖动平移、低灵敏度滚轮缩放、连线时输出端口到鼠标的临时跟随线，并将节点栏、属性栏、预览日志改为可移动/可隐藏/可浮动的 Dock 布局。Windows 构建目标设置为 GUI 子系统，正常启动时不显示终端窗口。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/main.cpp`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
GUI 增强保持在 `gui/` 层，不改变 `WorkflowGraph`、节点执行和 JSON 结构；连线最终仍通过 `WorkflowValidator` 校验后写入模型。

当前状态：
已解决

后续注意：
当前连线跟随线从输出端口开始，连接目标仍建议点输入端口，以避免多输入节点自动猜测错误端口。

---

### 2026-05-12 / GUI 启动方式修复阶段

类型：修改

概述：
修复 GUI 启动时额外显示终端窗口的问题。CMake 创建目标时显式声明 `WIN32` 和 `MACOSX_BUNDLE`，Windows 使用 GUI 子系统，macOS 生成 `.app` bundle；MinGW 额外链接 `-mwindows`，提高不同工具链下行为一致性。

影响范围：
`CMakeLists.txt`、`README.md`

处理方式：
保留原有 `--no-gui --workflow` 命令行执行入口，但普通 GUI 运行应使用生成的 GUI 程序或 `.app`，而不是从终端启动裸二进制。

当前状态：
已解决

后续注意：
如果从 IDE 启动仍弹终端，需要检查 IDE 的 Run in terminal / External console 选项；这属于 IDE 运行配置，不是程序自身窗口。

---

### 2026-05-12 / Apple 风格主题与界面缩放阶段

类型：修改

概述：
增加跨平台 Apple/Keynote 明亮玻璃风格主题和全局界面缩放功能。主题使用 Qt Widgets 自绘、stylesheet、半透明、圆角、渐变、阴影和 hover/pressed 状态模拟毛玻璃效果；全局缩放通过“视图”菜单和顶部工具栏提供界面放大、界面缩小、重置界面大小，并保存到 `QSettings`。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/gui/AppTheme.h`、`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
新增 `AppTheme` 集中管理颜色、尺寸、字体、stylesheet 和阴影；节点、端口、连线、画布背景改为按 `uiScale_` 矢量重绘；缩放后重建 scene 和属性面板，避免位图拉伸导致模糊。

当前状态：
已解决

后续注意：
当前毛玻璃为跨平台模拟效果，不使用 macOS vibrancy 或 Windows Acrylic/Mica；`third_party/qt-mvvm` 继续保持隔离，未链接到主程序。

---

### 2026-05-12 / ElaWidgetTools 可选迁移阶段

类型：修改

概述：
新增 `GuiCompat` 适配层和 CMake 可选集成入口。若 `third_party/ElaWidgetTools/ElaWidgetTools` 源码完整存在且 Qt 提供 `WidgetsPrivate`，主窗口将继承 `ElaWindow`，Dock、按钮、输入框、下拉框、复选框、数值输入等 UI 外壳控件会切换为 ElaWidgetTools 组件；否则继续使用当前 Qt Widgets 外壳。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/main.cpp`、`ImageNodeEditor/gui/ElaCompat.h`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
保持 `workflow/`、`nodes/`、`processing/`、JSON 格式和命令行模式不变；ElaWidgetTools 只作用于 GUI 层，避免把未验证第三方库变成无法回退的基础依赖。

当前状态：
已解决

后续注意：
ElaWidgetTools 当前需要 `Qt6::WidgetsPrivate`；CMake 会在缺失时自动回退。已对 vendored 源码补充 Qt 6 `QChar` 枚举转换兼容 helper，并在主项目 Ela 分支显式启用 AUTORCC。第三方头文件仍有若干 warning，不影响当前构建。

---

### 2026-05-12 / 图标化工具栏与设置入口阶段

类型：修改

概述：
将顶部工具栏动作改为图标优先显示，并增加 2 秒延迟中文悬停提示；新增“设置”入口和设置对话框，用于集中修改界面大小、画布缩放、Dock 面板显示状态，并提供重置界面大小和重置布局操作。

影响范围：
`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
图标使用 `QPainter` 在运行时绘制，不新增图片资源和打包路径；菜单项保留中文文本并增加图标，工具栏使用 `Qt::ToolButtonIconOnly`；设置项继续写入或复用现有 `QSettings`、Dock 和缩放逻辑。

当前状态：
已解决

后续注意：
后续新增软件级配置应优先放入设置对话框，避免继续分散在多个菜单和工具栏中。

---

### 2026-05-12 / 工具栏与主题设置完善阶段

类型：修改

概述：
将工具栏悬停提示延迟改为 1.5 秒并显示在鼠标指针附近；顶部工具栏固定在窗口顶部，缩小图标和按钮尺寸以接近 VS Code 风格；预览与日志默认迁移到右侧 Dock；设置窗口新增界面主题选项，支持浅色、深色和跟随系统。

影响范围：
`ImageNodeEditor/gui/AppTheme.h`、`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
主题状态集中在 `AppTheme`，通过 `QSettings` 保存；深色主题同步影响 stylesheet、画布、节点、连线和运行时绘制图标；默认 Dock 布局使用 `mainWindow/layoutVersion` 迁移旧布局。

当前状态：
已解决

后续注意：
ElaWidgetTools 仍会在编译时输出第三方 warning；当前不影响功能。

---

### 2026-05-12 / .gitignore 及编译残留清理阶段

类型：修改

概述：
发现仓库中残留了大量 cmake、C++编译生成的零散文件及目录（如 `CMakeFiles/`, `CMakeCache.txt`, `.qt/`, 以及因在根目录交叉编译导致的 `ElaWidgetTools/` 中间目录）。为防止污染仓库，将这些内容补充添加到 `.gitignore`。

影响范围：
`.gitignore`

处理方式：
在 `.gitignore` 首部补充了 C++ 与 CMake 编译产生的各类可执行文件、库文件和构建目录忽略规则。特别是标记了 `.qt/` 和由于内源构建错误产生的 `/ElaWidgetTools/` 文件夹。

当前状态：
已解决

后续注意：
后续请规范使用 `cmake -B build` 将构建产物统一放在独立存放目录，以免污染项目源码目录；现有因操作失误产生在项目根目录的编译目录及文件可直接手动删除，它们目前已被 Git 合法忽略。
