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

### 2026-06-09 / 顶部菜单与标题栏合并为单行阶段

类型：修改

概述：
Windows 上窗口顶部原为两行（第 1 行 `QMenuBar`，第 2 行 QML 标题栏）。将菜单并入 QML 标题栏最左侧，合并为单行：`菜单 | 导航键 | 标题框 | 右侧按钮`。

影响范围：
`ImageNodeEditor/gui/WorkbenchModels.h`/`.cpp`（`WorkbenchBridge`）、`ImageNodeEditor/gui/MainWindow.h`/`.cpp`、`ImageNodeEditor/qml/WorkbenchTitleBar.qml`；另修了 `ImageNodeEditor/GeneratedFiles/regen.bat`。

处理方式：
- `WorkbenchBridge` 新增 `headerMenuTitles` 属性、`openHeaderMenu(index, globalPos)` 方法、`headerMenuRequested` 信号（沿用既有「QML 调用→发信号→MainWindow 执行」中继模式）。
- `WorkbenchTitleBar.qml` 在导航键左侧用 `Repeater` 渲染菜单标题文字按钮（`MenuButton` 组件，复用命令按钮样式），点击调用 `openHeaderMenu(index, mapToGlobal(0, height))`，仅非 macOS 显示。
- `MainWindow` 建立 `headerMenus_`（按序映射 6 个现有原生 `QMenu`），连接 `headerMenuRequested` → `menu->popup()` 在按钮正下方弹出原生菜单——菜单内容（含动态「节点操作」子菜单）完全复用，不重写。
- 非 macOS 隐藏 `menuBar()`；macOS 仍用系统菜单栏、标题栏不显示菜单按钮。
- 坑（已修）：隐藏 `menuBar()` 后，菜单内动作的快捷键会失效。已把带快捷键的动作递归 `addAction` 到主窗口（仅非 macOS），复用同一 `QAction` 不产生歧义；实测 `Ctrl+K`（快捷添加节点）等正常触发。
- 工具：`GeneratedFiles/regen.bat` 原有两处 bug——`set "INC=…"` 外层引号与内层路径引号冲突、且中文注释在不匹配的控制台代码页下被当成命令——已改为 `set INC=`（去外层引号）并全部用 ASCII 注释，实测可正常重生成 moc/rcc。

当前状态：
已解决。原生构建实测：顶部为单行，6 个菜单点击均在按钮下方弹出对应原生菜单（含动态节点子菜单），菜单快捷键正常，拖动/最小化/最大化/关闭不受影响。

后续注意：
- 标题框居中沿用 QML 两个 `fillWidth` 占位符，左侧多了菜单按钮后略向右偏，属预期。
- 改了 `WorkbenchModels.h`（含 `Q_OBJECT`）或 `WorkbenchTitleBar.qml` 后，务必先运行 `GeneratedFiles/regen.bat` 再构建（原生工程用预生成 moc/rcc）。

---

### 2026-06-09 / Windows 无边框窗口阶段

类型：修改

概述：
Windows 上原生标题栏与程序自带的 QML 标题栏同时出现，右上角出现两套最小化/最大化/关闭按钮。实现 Windows 无边框窗口，隐藏原生标题栏，只保留程序自带操作区。

影响范围：
`ImageNodeEditor/gui/NativeWindowChrome.cpp`（仅 Windows 分支）。未改 `MainWindow`、`NativeWindowChrome_mac.mm`、QML 标题栏与 `WorkbenchBridge`。

处理方式：
此前 `NativeWindowChrome::configure()` 在非 macOS 上是空实现，故 Windows 原生标题栏没被隐藏。改为：在 Windows 分支用 `QAbstractNativeEventFilter` 处理 `WM_NCCALCSIZE`（把整窗作为客户区以去掉可见标题栏；最大化时按外框内缩，避免裁切并让任务栏可见）与 `WM_NCHITTEST`（在窗口边缘返回 HTLEFT/HTTOP… 实现拖边缩放）。保留窗口原生样式（不设 `FramelessWindowHint`），因此 Aero 贴边、最大化动画、投影都保持原生；投影另用 `DwmExtendFrameIntoClientArea` 保证。窗口拖动、最小化、最大化、关闭沿用既有 `WorkbenchBridge -> MainWindow`（`startSystemMove` / `showMinimized` / `showMaximized` / `close`）接线，无需改动。`dwmapi.lib` 用 `#pragma comment(lib, ...)` 自动链接，CMake 与原生 vcxproj 都无需改构建配置。

当前状态：
已解决。原生 VS 构建实测：原生标题栏消失、右上角仅剩程序自带按钮；拖动、最大化/还原、缩放均正常，最大化无裁切、任务栏可见。

后续注意：
- 仅 Windows 生效；macOS 仍由 `NativeWindowChrome_mac.mm` 处理，Linux 为空实现，三端均可编译。
- 自定义标题栏顶部约 6px 为缩放抓取带，与按钮的极窄重叠可接受；如需可在 QML 标题栏顶部留出对应留白。
- 自动隐藏任务栏在最大化时的弹出未特殊处理（少见场景）。

---

### 2026-06-09 / 原生 VS 工程改为由 VS 直接编译源码阶段

类型：修改

概述：
按用户要求，把根目录 VS 工程从“委托 CMake 的 Makefile 壳”改为**真正由 VS/MSVC 直接编译本项目源码**的原生 Application 工程。双击根目录 `ImageNodeEditor.slnx` → F5 即可编译运行，全程不经过 CMake、不需要 Qt VS Tools 扩展。

影响范围：
`ImageNodeEditor/ImageNodeEditor.vcxproj`（重写为 Application）、`.vcxproj.filters`、`.vcxproj.user`；新增 `ImageNodeEditor/qml/workbench.qrc`、`third_party/codicons/codicons.qrc`、`ImageNodeEditor/GeneratedFiles/`（预生成 moc/rcc + `regen.bat`）、`third_party/QtNodes/prebuilt/QtNodes.lib`；`README.md`。未改动任何 `ImageNodeEditor/` 业务源码与 `CMakeLists.txt`。

处理方式：
- moc/rcc 采用**预生成**策略：用 Qt 6.8.3 的 moc 处理 3 个含 `Q_OBJECT` 的头（`CanvasTabBar.h`、`WorkbenchHostWidget.h`、`WorkbenchModels.h`），rcc 处理两份资源（`/workbench` QML、`/codicons` 字体），产物放入 `GeneratedFiles/` 作为普通 `.cpp` 编译，使 VS 构建不依赖命令行 Qt 工具。`regen.bat` 供改动后刷新。
- 第三方 QtNodes 用 CMake 预编译的 `QtNodes.lib` 链接（放 `third_party/QtNodes/prebuilt/`，避开 `.gitignore` 的 `lib/` 规则）；因其为静态库且内置默认样式资源，链接器加 `/WHOLEARCHIVE:QtNodes.lib` 防止资源自注册被裁剪；编译期需定义 `NODE_EDITOR_STATIC`（否则 `Export.hpp` 报“Choose whether to link against shared or static”）。
- 链接 Qt6 模块导入库（Core/Gui/Widgets/OpenGL/Qml/Quick/QuickControls2/QuickWidgets + EntryPoint），`/MD`、C++17、`/permissive-`、`/Zc:__cplusplus`、`/utf-8`；GUI 子系统。
- PostBuild 调 `windeployqt` 自动部署运行库到 `out/Release/`。

当前状态：
已解决。命令行 `MSBuild ImageNodeEditor.slnx /p:Configuration=Release /p:Platform=x64`（不调用 CMake）实测编译链接通过；CLI（`version`）与 GUI 均成功运行。

后续注意：
- 工具集 v145、Windows SDK 10.0.26100.0、仅 `Release|x64`。Debug 需另装 debug 版 Qt。
- moc/rcc 是预生成快照：新增/修改 `Q_OBJECT` 类或改 qml 后须运行 `GeneratedFiles/regen.bat`（否则 VS 里会缺符号/资源）；日常活跃开发也可继续用 CMake 构建（AUTOMOC 自动处理）。
- `QtNodes.lib` 为 Release/x64/MSVC 专用预编译产物；升级 Qt 或换工具链需用 CMake 重新生成后替换。
- `out/` 已 gitignore；`GeneratedFiles/` 与 `prebuilt/QtNodes.lib` 需随提交保留。
- 坑（已修）：PostBuild 的 windeployqt 起初没带 `qmldir` 选项，未部署 Qt Quick/QML 模块（QtQuick / Controls / Layouts / QtQml），导致工作台 QML 侧边栏（节点库、活动栏）整块**黑屏**。已在 PostBuild 加 `--qmldir "$(ProjectDir)qml"`，让 windeployqt 扫描 qml 源码并部署所需模块到 `out\Release\qml\`。注意 vcxproj 的 XML 注释里不能出现连续两个连字符（会触发 MSB4025），描述该选项时改用“qmldir 选项”等措辞。

---

### 2026-06-09 / 新增原生 VS 2026 提交工程结构阶段

类型：修改

概述：
按课程提交要求，在仓库中新增原生 Visual Studio 2026 解决方案布局：根目录 `ImageNodeEditor.slnx`（新版 XML 解决方案格式），同名项目目录 `ImageNodeEditor/` 下新增 `ImageNodeEditor.vcxproj` / `.vcxproj.filters` / `.vcxproj.user`，`main.cpp` 已在该目录内。未改动任何源码、`CMakeLists.txt` 或 `third_party/`。

影响范围：
`ImageNodeEditor.slnx`（新增）、`ImageNodeEditor/ImageNodeEditor.vcxproj` / `.filters` / `.user`（新增）、`README.md`。

处理方式：
`.vcxproj` 采用 **Makefile（NMake）类型**，把“生成”动作委托给现有 CMake（`cmake --build build-vs --config Release`，首次自动配置 `build-vs/`），即真正编译仍走 CMake，避免在 VS 与 CMake 间维护两份编译配置，也无需安装 Qt VS Tools 扩展（该扩展对全新 VS 2026/v18 兼容性未知）。`.filters` 按 app/core/gui/nodes/processing/util/workflow/qml 镜像目录树；`.user` 把 F5 调试目标指向 CMake 产物并把 Qt bin 注入 PATH。Qt 路径用属性 `QtDir`（默认 `C:\Qt\6.8.3\msvc2022_64`，可由环境变量 `QTDIR` 覆盖）。本机仅装 Release 版 Qt，故只提供 `Release|x64` 单配置。

当前状态：
已解决。命令行 `MSBuild ImageNodeEditor.slnx /p:Configuration=Release /p:Platform=x64` 实测通过：自动配置 CMake、编译出 `QtNodes.lib` 与 `ImageNodeEditor.exe`。

后续注意：
- 跨平台与 macOS 仍以根目录 `CMakeLists.txt` 为准；原生 `.vcxproj` 是 Windows 专用、提交结构用，不参与 macOS 构建。
- 该原生工程依赖 `build-vs/` 由 CMake 配置（NMake 命令里已含“缺失则自动配置”）；换机器时若 Qt 路径不同，改 `QtDir` 默认值或设 `QTDIR`。
- 若日后需要 VS 内真正的 Debug 单步，需用 aqt 另装 debug 版 Qt 并在 `.vcxproj` 增加 `Debug|x64` 配置。
- 坑：CMake 在 `build-vs/` 里也会生成一个**同名** `ImageNodeEditor.slnx`，且其默认启动项目是不可运行的 `ALL_BUILD`，直接 F5 会报“无法启动程序 …ALL_BUILD / 拒绝访问”。已在 `CMakeLists.txt` 加 `set_property(DIRECTORY ... PROPERTY VS_STARTUP_PROJECT ImageNodeEditor)`，把生成解决方案的启动项目设为 `ImageNodeEditor`，两个 slnx 现在 F5 均可运行。日常/提交请开**根目录**的 `ImageNodeEditor.slnx`；`build-vs/` 已加入 `.gitignore`。

---

### 2026-06-09 / Windows 首次搭建 Qt 环境并用 VS 2026 编译阶段

类型：决策 / 修改

概述：
本机原本只有 Visual Studio 2026 Community（v18.6.2，MSVC 14.51，自带 CMake 4.2.3 + Ninja），没有 Qt，也没有可用的 Python（仅 Microsoft Store 占位 alias）。为在 Windows 上重新编译出 `.exe`，新引入开发环境依赖并完成一次完整 Release 构建与运行验证。

影响范围：
仅开发环境与 `build-vs/` 构建产物，未改动任何源码、`CMakeLists.txt`、JSON 格式或节点接口。

处理方式：
1. winget 安装 Python 3.12（消除 Store alias），`pip install --user aqtinstall`。
2. `aqt install-qt windows desktop 6.8.3 win64_msvc2022_64`（基础包已含 qtdeclarative，即 Qml/Quick/QuickControls2/QuickWidgets），再补 `-m qtshadertools`（Qt Quick 在 CMake 配置时会传递性要求它）。安装到 `C:\Qt`。VS 2026 的 MSVC 与 Qt 的 msvc2022_64 二进制 ABI 兼容。
3. 用 VS 自带 cmake 配置：`cmake -S . -B build-vs -G "Visual Studio 18 2026" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64`，生成 `.sln`；`cmake --build build-vs --config Release` 通过。
4. `windeployqt --release` 部署运行期 DLL 与插件。

当前状态：
已解决。CLI（`version`/`nodes` 等）与 GUI 均成功运行。

后续注意：
- `windeployqt` 默认只部署 `platforms/qwindows.dll`。本项目命令行模式在 `main.cpp` 里运行时设 `QT_QPA_PLATFORM=offscreen`，windeployqt 无法感知，导致缺 `platforms/qoffscreen.dll`，CLI 启动即 `0xC0000409`（平台插件加载失败 abort）。需手动从 `C:\Qt\6.8.3\msvc2022_64\plugins\platforms\qoffscreen.dll` 复制到部署目录；GUI 模式用 qwindows，不受影响。
- 构建尾部 `'pwsh.exe' 不是内部命令` 与 windeployqt 的 `dxcompiler/dxil.dll`、`VCINSTALLDIR` 警告均无害，不影响产物。

---

### 2026-05-21 / Qt GUI 候选原型验证阶段

类型：修改

概述：
新增隔离的 Qt/C++ GUI 候选实验层，分别用 Qlementine、Qt Nodes 和 Qt Advanced Docking System 验证现代控件壳、节点画布和专业停靠工作台方向；同时落库候选来源、许可证记录、构建命令、截图和决策报告。

影响范围：
`CMakeLists.txt`、`experiments/gui_prototypes/`、`third_party/Qlementine/`、`third_party/QtNodes/`、`third_party/QtAdvancedDockingSystem/`、`third_party/GUI_PROTOTYPE_SOURCES.md`

处理方式：
主目标新增默认关闭的 `IMAGENODEEDITOR_BUILD_GUI_PROTOTYPES` 开关，实验目标复用最小工作台占位数据而不接入现有 workflow JSON 与执行引擎；第三方源码使用固定 vendored 快照，原始许可证文件保留在各目录根部。

当前状态：
已解决

后续注意：
正式 UI 路线建议先落视觉壳与布局质量改进，再决定是否承担 Qt Nodes 适配层或新 docking 系统的迁移成本；将任何候选链接进主程序前需要再次核对 Windows 构建和许可证影响。

---

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

---

### 2026-05-12 / GUI 交互微调阶段

类型：修改

概述：
修改顶部工具栏图标悬停提示（ToolTip）的延迟触发时间。

影响范围：
`ImageNodeEditor/gui/MainWindow.cpp` (`DelayedToolTipFilter`)

处理方式：
将 `timer_.setInterval(1500);` 缩短为 `timer_.setInterval(500);`，从而将悬停提示的延迟时间从 1.5 秒减少到 0.5 秒。并在 `build/` 目录中重新编译验证通过。

当前状态：
已解决

后续注意：
若提示干扰到其他高频操作，可使用选项设置窗口进行参数抽离或后期调整。

---

### 2026-05-12 / 执行性能与状态反馈阶段

类型：修改

概述：
实现节点级增量执行缓存和画布节点运行状态高亮。`ExecutionEngine` 现在维护内存缓存，基于节点类型、参数、输入连线、上游签名和读图文件信息生成签名；连续执行未变化 workflow 时可复用缓存。GUI 执行时节点会显示正在执行、成功、失败和复用缓存状态，失败节点会自动选中并居中。

影响范围：
`ImageNodeEditor/workflow/ExecutionEngine.h`、`ImageNodeEditor/workflow/ExecutionEngine.cpp`、`ImageNodeEditor/nodes/ImageNode.h`、`ImageNodeEditor/nodes/ImageNode.cpp`、`ImageNodeEditor/nodes/BasicNodes.cpp`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
新增 `ImageNode::isCacheable()`，默认节点可缓存，`ImageOutput` 标记为不可缓存，避免导出被跳过。`MainWindow` 持有同一个 `ExecutionEngine`，新建或打开 workflow 时清空缓存；参数、节点和连线修改会重置节点显示状态，但由签名负责实际缓存失效判断。

当前状态：
已解决

后续注意：
缓存仅保存在内存中，不写入 workflow JSON；当前执行仍是同步执行，后续实时预览或并行计算应继续复用节点摘要和缓存接口。

---

### 2026-05-12 / 撤销重做机制阶段

类型：修改

概述：
实现 GUI 编辑操作的撤销 / 重做机制。新增 `QUndoStack` 和 `WorkflowCommands` 快照命令层，添加节点、复制节点、删除节点/连线、添加连线、移动节点和修改参数都进入撤销栈；窗口标题显示未保存状态，新建、打开和关闭时会提示保存未保存修改。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`ImageNodeEditor/gui/WorkflowCommands.h`、`ImageNodeEditor/gui/WorkflowCommands.cpp`、`plan.md`

处理方式：
采用深拷贝 workflow 快照恢复图模型，避免在现有 `MainWindow` 集中交互逻辑上大范围重构。参数修改和节点移动使用 merge key 合并连续操作；保存成功后调用 `undoStack->setClean()`，新建/打开 workflow 时清空撤销栈。

当前状态：
已解决

后续注意：
当前撤销命令采用整图快照，适合当前项目规模；如果后续出现超大 workflow，可再替换为更细粒度命令。当前本机默认 `build/` 缓存存在 x86_64/arm64 架构不匹配，已用临时 arm64 构建目录验证通过。

---

### 2026-05-12 / macOS 启动依赖修复阶段

类型：修改

概述：
修复 macOS `.app` 无法正常打开的问题。原因是主程序链接了 `@rpath/libElaWidgetTools.dylib`，但 app 包内没有复制该动态库，dyld 启动时找不到依赖导致程序无法启动。

影响范围：
`CMakeLists.txt`、`build/` 重新配置和清理构建产物

处理方式：
在启用 ElaWidgetTools 且为 Apple 平台时，为 `ImageNodeEditor` 设置 app bundle 的 rpath，并在 `POST_BUILD` 阶段把 `$<TARGET_FILE:ElaWidgetTools>` 复制到 `ImageNodeEditor.app/Contents/Frameworks/`。同时默认 macOS 架构使用 `CMAKE_HOST_SYSTEM_PROCESSOR`，避免 Homebrew arm64 Qt 与旧 x86_64 构建缓存混用。

当前状态：
已解决

后续注意：
如果旧构建目录曾生成过不同架构的对象文件，需要先执行 `cmake --build build --target clean` 再重新构建。ElaWidgetTools 第三方 warning 不影响启动。

---

### 2026-05-12 / 快捷节点面板阶段

类型：修改

概述：
实现快捷节点面板，支持通过当前平台主快捷键和画布 `Tab` 快速搜索并添加节点，减少从节点栏拖动或双击添加的操作成本。

影响范围：
`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
新增轻量弹出式节点搜索面板，搜索数据直接来自 `NodeFactory::descriptors()`，支持按中文显示名、英文类型名和分类过滤。通过 `Cmd/Ctrl+K` 或画布 `Tab` 打开时，节点会创建在鼠标所在画布位置；鼠标不在画布内时创建在视口中心。右键画布空白处也可打开快捷添加入口。实际创建仍复用 `addNodeFromType()`，因此保留自动避让、撤销/重做和运行状态重置。

当前状态：
已解决

后续注意：
当前面板只负责搜索和创建节点，不承担连线或参数编辑；后续连线磁吸和数据类型可视化仍按 `plan.md` 继续实现。

---

### 2026-05-12 / 连线磁吸与反馈阶段

类型：修改

概述：
实现连线过程中的输入端口磁吸与视觉反馈。用户从输出端口开始连线后，合法输入端口会显示可连接反馈；靠近合法输入端口时临时连线自动吸附到端口中心，松开或点击即可优先连接；靠近非法输入端口时显示红色禁用反馈，避免误连。

影响范围：
`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
在 `WorkflowScene` 中维护当前吸附端口和端口反馈状态，吸附距离随 `uiScale` 缩放。合法性判断复用 `WorkflowValidator::validateEdge()`，因此端口类型、单输入限制和环检测都沿用模型层规则。`PortItem` 增加轻量连接反馈状态，绘制时通过颜色、光晕和轻微放大表达可连接、禁用和已吸附状态。

当前状态：
已解决

后续注意：
当前仍兼容原来的“先点输出端口、再点输入端口”操作；下一步数据类型可视化可以继续细化端口基础颜色，不需要重写连线校验。

---

### 2026-05-12 / 数据类型可视化阶段

类型：修改

概述：
实现端口数据类型可视化。端口基础颜色现在由 `PortType` 决定，而不是只按输入 / 输出方向区分；用户可以通过颜色快速识别图像、数值、文本、Mask 和图片列表端口。

影响范围：
`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
新增端口类型到颜色的映射：`ImageRGBA` / `ImageGray` 使用蓝色，`Number` 使用绿色，`Text` 使用紫色，`Mask` 使用橙色，`ImageList` 使用青色。端口悬停提示补充方向和数据类型。连线拖拽时继续复用 `WorkflowValidator::validateEdge()` 给出可连接、吸附或禁用反馈，避免 GUI 层重复实现连接规则。

当前状态：
已解决

后续注意：
端口颜色现在表达数据类型，连线方向仍通过端口所在节点左右位置和悬停提示表达；如果后续引入更多类型，应同步扩展 `portColor()`。

---

### 2026-05-12 / 日志追溯阶段

类型：修改

概述：
实现一键日志追溯。GUI 日志现在不再只是纯文本追加，而是带节点元数据的日志列表；用户双击执行、缓存、成功、失败等节点相关日志时，画布会自动定位并选中对应节点。

影响范围：
`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
将日志控件从只读文本框改为 `QListWidget`，`appendLog()` 支持可选 `nodeId` 元数据并通过 `Qt::UserRole` 保存。工作流执行日志改为根据 `ExecutionResult::nodeSummaries` 写入，避免 GUI 再解析纯文本日志。双击日志项时调用 `centerOn()`、选中目标节点、刷新属性和预览，并绘制短暂虚线高亮框。

当前状态：
已解决

后续注意：
没有节点关联的普通日志仍可显示，但双击不会触发定位；被删除节点对应的历史日志也会安全忽略。

---

### 2026-05-12 / 实时预览阶段

类型：修改

概述：
实现参数修改后的防抖实时预览。用户修改当前选中节点参数后，程序会在短暂停顿后自动执行该节点及其上游依赖，并刷新预览区域。

影响范围：
`ImageNodeEditor/workflow/ExecutionEngine.h`、`ImageNodeEditor/workflow/ExecutionEngine.cpp`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
`ExecutionEngine` 新增 `executeForNode()`，根据目标节点反向收集上游依赖，只执行目标节点和必要上游节点，避免实时预览触发下游 `ImageOutput` 等副作用节点写文件。`MainWindow` 增加 350ms 单次 `QTimer` 防抖，在参数修改后自动调度预览；成功时刷新预览图，失败时只写入带节点定位能力的日志，不弹出阻塞对话框。

当前状态：
已解决

后续注意：
实时预览仍在主线程同步执行，依赖增量缓存降低重复计算成本；后续若实现并行计算或后台任务，可继续复用 `executeForNode()` 的局部执行边界。

---

### 2026-05-12 / 无依赖分支并行执行阶段

类型：修改

概述：
实现无依赖 DAG 分支的并行执行。执行引擎现在会按依赖关系分层调度，同一轮中互不依赖的就绪节点会并行执行，下游节点等待所有必要上游完成后再进入下一轮。

影响范围：
`ImageNodeEditor/workflow/ExecutionEngine.cpp`、`plan.md`

处理方式：
将 `executeOrderedNodes()` 从简单顺序循环改为入度调度：构建当前执行范围内的入度和下游表，每轮收集入度为 0 的 ready 节点。缓存命中的节点直接复用输出；需要计算的节点通过 `std::async(std::launch::async)` 执行，主执行线程统一收集结果、更新缓存、记录摘要并触发 GUI 回调。这样缓存 `QMap` 和 UI 回调仍只在主执行线程访问，避免数据竞争。

当前状态：
已解决

后续注意：
当前并行粒度按就绪层控制，失败时不再调度下游节点；已启动的同层节点会自然完成后统一收集结果。后续如需取消长耗时节点，可再扩展节点执行接口的取消机制。

---

### 2026-05-12 / 框选与批量操作阶段

类型：修改

概述：
实现框选和批量节点操作。用户可以在画布空白处按住 `Shift` 拖拽进行框选，多选后可整体移动、批量删除，也可以批量复制节点。

影响范围：
`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
在 `ZoomGraphicsView` 中为 `Shift + 左键拖拽空白区域` 启用 `QGraphicsView::RubberBandDrag`，释放后恢复原有无拖拽模式，保留普通左键空白拖动画布平移。删除逻辑沿用现有多选删除流程。复制逻辑扩展为读取当前选中的所有 `NodeItem`，逐个复制节点类型和参数，使用 `findAvailableNodePosition()` 避让重叠，默认不复制外部连线，并将批量复制作为一个撤销命令记录。

当前状态：
已解决

后续注意：
当前批量复制只复制节点本体和参数，不复制原节点之间的内部连线；如果后续需要完整子图复制，可在此基础上增加选区内连线重建。

---

### 2026-05-12 / 缩略小地图阶段

类型：修改

概述：
实现画布缩略小地图。画布左下角现在显示全局节点分布和当前视口位置，节点较多时可快速判断当前位置，并支持点击或拖拽小地图快速移动视口。

影响范围：
`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
新增轻量 `MiniMapWidget` 自绘控件，挂载在画布 `QStackedLayout` 左下角。小地图根据当前 workflow 节点位置和 `QGraphicsView` 可见区域计算世界范围，绘制节点矩形和视口框；点击或拖拽时反算场景坐标并调用 `centerOn()`。节点移动、重建、滚动和缩放时都会触发小地图刷新。

当前状态：
已解决

后续注意：
当前小地图默认显示，尺寸随 UI 缩放变化；如后续需要设置开关，可在现有设置对话框里增加一个可见性选项。

---

### 2026-05-12 / 画布截图导出阶段

类型：修改

概述：
实现画布拓扑截图导出。用户可以通过文件菜单、主工具栏或快捷键将当前完整 workflow 拓扑导出为高清 PNG，用于汇报、文档或作业展示。

影响范围：
`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
新增“导出画布截图”动作和图标，快捷键为 `Cmd/Ctrl+Shift+E`。导出时使用 `QFileDialog` 选择 PNG 路径，基于 `QGraphicsScene::itemsBoundingRect()` 计算完整拓扑范围，临时清除选择状态后用 `QGraphicsScene::render()` 以 2x 分辨率渲染到 `QImage`，保存失败时给出明确提示，成功后写入日志。

当前状态：
已解决

后续注意：
导出内容只包含节点与连线拓扑，不包含画布上的小地图和缩放按钮等悬浮控件；当前背景由导出逻辑直接绘制渐变底色。

---

### 2026-05-13 / 宏节点与子图阶段

类型：修改

概述：
实现宏节点 v1。用户可以将选中的节点封装为宏节点，外部输入/输出端口由跨选区连线自动推导，双击宏节点可进入内部子图编辑，返回上级图后子图内容会保存回宏节点。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/nodes/MacroNode.h`、`ImageNodeEditor/nodes/MacroNode.cpp`、`ImageNodeEditor/nodes/NodeFactory.cpp`、`ImageNodeEditor/workflow/ExecutionEngine.h`、`ImageNodeEditor/workflow/ExecutionEngine.cpp`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
新增 `MacroNode` 并注册到节点工厂，宏节点通过 `saveParams()` / `loadParams()` 序列化内部 `WorkflowGraph` 和端口映射。执行引擎新增 `executeWithExternalInputs()`，宏节点执行时把外部输入注入内部子图，再把映射的内部输出暴露为宏节点输出。GUI 新增“封装为宏节点”和“返回上级图”，当前画布可切换到宏节点子图上下文，保存 workflow 时会从当前嵌套上下文回写到根图。

当前状态：
已解决

后续注意：
宏节点端口当前由封装瞬间的跨选区连线生成，暂不提供手动端口编辑器；进入/退出子图时仍以当前整图快照机制保持数据一致。

---

### 2026-05-13 / 小地图设置阶段

类型：修改

概述：
补齐小地图显示设置。现有小地图已支持全局视图、视口框以及点击/拖拽定位，现在设置窗口中可以显式开启或隐藏小地图。

影响范围：
`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
在设置对话框新增“显示小地图”复选项，使用 `QSettings` 持久化 `mainWindow/showMiniMap`，默认开启。重置布局时会恢复小地图显示并同步设置。

当前状态：
已解决

后续注意：
小地图仍复用现有 `MiniMapWidget` 实现，没有引入额外视图或模型层状态。

---

### 2026-05-13 / 自动布局阶段

类型：修改

概述：
实现一键自动布局。布局菜单和工具栏新增“自动布局”，可将当前图按数据流从左到右规整排列，宏节点按普通节点处理，进入宏节点子图后只作用于当前子图。

影响范围：
`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
自动布局前先校验现有连线端点、端口、类型和单输入约束，再通过 `WorkflowValidator::topologicalOrder()` 检测环。校验通过后计算 DAG 层级，同层节点纵向排列，布局修改进入撤销栈；校验失败时弹窗说明原因且不修改当前布局。

当前状态：
已解决

后续注意：
当前布局算法是确定性的层级布局，不引入物理弹簧或长连线最小化优化。

---

### 2026-05-13 / 执行诊断反馈阶段

类型：修改

概述：
实现节点执行耗时和运行中动画反馈。节点执行时状态条会流动，执行完成后节点标题区域显示最近一次耗时；缓存命中显示“缓存”，避免误报真实执行耗时。

影响范围：
`ImageNodeEditor/workflow/ExecutionEngine.h`、`ImageNodeEditor/workflow/ExecutionEngine.cpp`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
`NodeExecutionSummary` 增加 `elapsedMs`，执行引擎用 `QElapsedTimer` 统计真实节点执行耗时，缓存命中记录为 `0ms`。GUI 保存节点最近耗时并传给 `NodeItem` 绘制；运行中节点由 `QTimer` 驱动状态条动画，所有节点结束后自动停止刷新。

当前状态：
已解决

后续注意：
本阶段按计划只实现“耗时 + 动画”，悬浮连线数据预览未纳入本轮范围。

---

### 2026-05-13 / 跨平台菜单与深色模式修复阶段

类型：修改

概述：
保留菜单栏与工具栏双入口，并修复深色模式下主窗口、Dock 区域、画布外露区域、滚动区域和设置窗口的浅色残留。

影响范围：
`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
菜单栏继续使用 Qt `QMenuBar`，并在代码中说明 macOS 会显示为系统菜单、Windows/Linux 会显示在窗口内，底层 `QAction` 跨平台复用。主题层扩展深色 `QPalette` 角色并同步 ElaWidgetTools 的主题模式；样式表补充 `QMainWindow::separator`、`QSplitter::handle`、`QDialog`、`QAbstractScrollArea`、`QGraphicsView`、Dock 内容区和日志列表等覆盖。主窗口刷新 UI 缩放时显式给画布 view 与 viewport 设置当前主题背景，避免 scene 外侧露出白底。

当前状态：
已解决

后续注意：
macOS 顶部系统菜单是 Qt 的平台原生行为，不作为缺陷处理；Windows/Linux 仍会在窗口内显示菜单。

---

### 2026-05-13 / 节点编辑器界面重排与真实背景采样节点阶段

类型：修改

概述：
按新的界面计划调整工具栏分组、放大主要控制按钮、压缩并放大左侧节点列表，并把节点绘制改为所有主题下都基于画布背景采样的磨砂玻璃效果。右侧属性 Dock 默认移除，节点参数编辑迁移到选中节点内部展开区域。

影响范围：
`ImageNodeEditor/gui/AppTheme.h`、`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
工具栏保留菜单与快捷入口，返回上级图动作移到独立导航工具栏右侧。节点列表字体加大、行高收紧。`NodeItem::paint()` 采样同一套画布背景网格到离屏图像并做轻量模糊，再叠加主题自适应半透明玻璃层、边框和高光，避免旧的银色固定渐变。节点参数通过 `QGraphicsProxyWidget` 嵌入选中节点内部，参数修改仍复用现有撤销、运行状态重置和实时预览路径。

当前状态：
已解决

后续注意：
本阶段按要求只采样画布背景，不采样其他节点或连线，以避免递归绘制和性能风险。

---

### 2026-05-13 / 顶栏交互与工作簿标签阶段

类型：修改

概述：
按最新 `plan.md` 优化顶栏和节点交互：工具栏按钮改为更紧凑的纯图标显示，节点文字比例加大；选中节点自动提升到最高图层；日志面板增加一键清空按钮；右键已选节点时保留多选集合，可直接封装宏节点；新增工作簿标签页，在顶栏以类似浏览器标签的方式切换多个画布。

影响范围：
`ImageNodeEditor/gui/AppTheme.h`、`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`plan.md`

处理方式：
在主题层收紧 `QToolBar` / `QToolButton` 的 padding、spacing 和最小尺寸，同时增加节点标题与标签字号。`NodeItem` 选中时提升 `zValue`，取消选中后恢复默认层级。画布右键菜单在命中已选节点时不再清空其它选中节点，使宏节点封装能作用于当前多选集合。底部日志区域增加清空按钮。工作簿使用 `QTabBar` 保存每个标签的 `WorkflowGraph`、选中节点、文件路径和脏状态，切换时同步当前画布快照并加载目标画布。

当前状态：
已解决

后续注意：
工作簿为内存级多画布切换，不改变 workflow JSON 格式，也不跨程序启动持久化多个标签页。

---

### 2026-05-13 / VS Code 风格工作区重排阶段

类型：修改

概述：
按最新界面计划重排工作区：删除节点列表侧栏，新增顶部“节点操作”菜单和右侧更多操作按钮；右侧只保留预览；底部改为“终端 / 问题 / 输出”标签面板，并把深色主题改为黑灰配色。

影响范围：
`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
节点添加入口改为基于 `NodeFactory::descriptors()` 动态生成的“节点操作”菜单，更多操作菜单复用同一批节点命令并补充文件、执行、视图和设置入口。原节点列表 Dock 被移除，Dock 布局版本升级，避免恢复旧布局。底部面板使用标签页承载 `QProcess` 命令运行器、问题列表和输出日志；终端默认使用 macOS/Linux 的 `$SHELL` 或 Windows 的 PowerShell/cmd。深色主题切换到 `#121314`、`#191A1B`、`#2E2E2F` 等黑灰色，节点和画布背景同步去除深蓝主色。

当前状态：
已解决

后续注意：
终端当前是跨平台命令运行器，不是完整 PTY/ConPTY 终端；适合普通命令和构建命令，不承诺全屏 TUI 程序的完整交互。

---

### 2026-05-15 / 全应用黑白灰单色化与直角化阶段

类型：修改

概述：
将应用视觉系统统一收敛为黑白灰，并把所有矩形类控件与自绘节点改为直角表现。浅色主题中的淡蓝按钮、选中、悬停和焦点反馈已全部替换为灰阶；深色主题继续保留黑灰底色，不再使用彩色强调。

影响范围：
`ImageNodeEditor/gui/AppTheme.h`、`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
主题颜色、调色板、菜单、按钮、列表、焦点态和滚动条统一改为灰阶值，工具栏图标、端口、连线、小地图、问题提示和节点高亮也同步去色。QSS 中矩形元素的圆角全部归零，自绘节点、状态条、耗时徽章、小地图框和图标内部矩形改为标准矩形。节点继续保留真实背景采样与模糊玻璃材质，但从圆角玻璃改为直角玻璃；执行中、成功、失败、缓存状态改由文字、灰阶亮度、边框和动画区分。

当前状态：
已解决

后续注意：
颜色参数节点仍保留业务上的颜色输入能力；本阶段限制的是应用界面视觉系统，而不是图像处理节点本身可使用的数据类型。

---

### 2026-05-15 / 顶部栏重组与跨平台导航阶段

类型：修改

概述：
把原先分散的多条顶栏收敛为高频命令栏、窗口标题层和次级工作簿栏，并补充 workflow 副本导出、预览结果导出与宏节点前进导航。

影响范围：
`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`、`ImageNodeEditor/gui/AppTheme.cpp`

处理方式：
高频命令栏按指定顺序重排文件与导出动作，并把缩放动作推到右侧。macOS 继续使用原生系统菜单；Windows/Linux 通过窗口标题层复用同一批 `QMenu`，左侧显示菜单、中央显示文件名和脏状态、右侧显示预览/底部面板/设置图标。工作簿栏保留标签，并在最右侧增加绿色执行按钮。宏节点导航新增前进历史，后退后可回到刚离开的子图。导出预览结果依赖当前预览图缓存，避免无内容时误导用户。

当前状态：
已解决

后续注意：
本阶段在灰阶主题中只保留两类颜色例外：绿色执行按钮，以及失败节点、失败连线和问题文本使用的红色反馈。

---

### 2026-05-21 / VS Code 风格工作台与分级清理阶段

类型：修改

概述：
把主工作区从预览与底部面板依赖 Dock 的布局收敛为固定工作台：左侧 Activity Bar、节点/工作流主侧栏、中央画布、右侧预览次栏和底部诊断面板。同步清理仓库内已确认无引用的生成物和系统杂项。

影响范围：
`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/gui/MainWindow.h`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
主窗口使用水平和垂直 `QSplitter` 管理工作台区域尺寸，节点库侧栏提供搜索和双击创建，工作流侧栏列出当前图节点并可定位。预览和底部面板的显示隐藏动作改为工作台区域动作，布局恢复改为保存 splitter 状态，不再暴露旧 Dock 移动入口。清理阶段移除了 `nodeeditor/build`、空 `output` 目录以及可见 `.DS_Store`，保留仍在构建边界或文档引用中的第三方目录和 `Problem.html`。

当前状态：
已解决

后续注意：
本轮只做可证明安全的清理；`qt-mvvm` 等未链接第三方目录若要移除，应单独确认子模块和参考代码用途后再执行。

---

### 2026-05-22 / Qt Nodes 主画布迁移与依赖收敛阶段

类型：修改

概述：
将主程序画布迁移到 Qt Nodes，保留 `WorkflowGraph`、执行引擎、缓存、宏节点和 workflow JSON 作为业务内核；同时移除旧自绘画布路径和未选中的 GUI 第三方路线。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/gui/MainWindow.*`、`ImageNodeEditor/gui/WorkflowCanvas.*`、`ImageNodeEditor/gui/WorkflowNodeDelegate.*`、`ImageNodeEditor/gui/WorkflowNodePainter.*`、`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/main.cpp`、`struct.md`、`third_party/QtNodes/`

处理方式：
新增 Qt Nodes 画布桥接层，把节点、端口、连线、选中、移动、参数控件和执行状态映射到 Qt Nodes，而连线合法性、撤销快照和持久化仍回写现有图模型。节点 painter 按输入输出、几何、色彩、滤波、合成和高级功能类别绘制不同轮廓与分类色。主入口退回标准 Qt Widgets，删除 Ela 兼容层和旧 `NodeItem` / `PortItem` / `EdgeItem` / 旧拖线场景实现；清理 ElaWidgetTools、Qlementine、ADS、`qt-mvvm`、未选原型及 Qt Nodes 未构建的 examples/tests/docs。

当前状态：
已解决

后续注意：
Qt Nodes 只负责 GUI 承载，不替代 `WorkflowSerializer`。后续若升级 vendored Qt Nodes，需要同步来源记录并重新验证宏节点导航、节点内参数和连接校验回写。

---

### 2026-05-23 / Qt Quick 混合工作台骨架阶段

类型：修改

概述：
在保留现有 C++ workflow 内核和 Qt Nodes 画布的前提下，为主窗口接入 Qt Quick 工作台壳层。新界面以 QML 承载 Activity Bar、主侧栏、状态栏和 Quick Access，中央画布、右侧预览和底部终端/问题/输出 Panel 仍复用现有 QWidget。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/main.cpp`、`ImageNodeEditor/gui/MainWindow.*`、`ImageNodeEditor/gui/WorkbenchHostWidget.*`、`ImageNodeEditor/gui/WorkbenchModels.*`、`ImageNodeEditor/qml/`、`struct.md`

处理方式：
主目标新增 Qt QML、Quick、Quick Controls 2 和 Quick Widgets 模块，并固定 Qt Quick Controls 为可定制的 `Basic` 样式。`WorkbenchHostWidget` 用 `QQuickWidget` 嵌入工作台表面，用 splitter 继续承载 Qt Nodes 编辑器、预览和底部 Panel。命令面板复用现有 `QAction`；节点目录、大纲、问题列表和最近 workflow 通过 C++ 列表模型暴露给 QML，QML 只把节点创建、节点定位、区域显示切换和最近 workflow 打开请求回传到 `MainWindow`。

当前状态：
已解决

后续注意：
这是混合迁移的第一阶段。底部 Panel、预览内容和工作簿标签仍由 Widgets 实现；后续迁移应继续沿 `WorkbenchBridge` 边界接入，不要把 `WorkflowGraph` 修改逻辑搬到 QML。

---

### 2026-05-24 / VS Code Dark Qt Quick 工作台重写阶段

类型：修改

概述：
将主界面收敛为只保留深色模式的 VS Code Dark 风格工作台：QML 自定义标题栏、Activity Bar、主侧栏、编辑区头部、右侧预览、底部 Panel、状态栏和 Quick Access 统一接入，旧工具栏和旧窗口标题层不再作为可见 UI。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/gui/AppTheme.*`、`ImageNodeEditor/gui/MainWindow.cpp`、`ImageNodeEditor/gui/WorkbenchHostWidget.*`、`ImageNodeEditor/gui/WorkbenchModels.*`、`ImageNodeEditor/gui/WorkflowCanvas.*`、`ImageNodeEditor/gui/WorkflowNodeDelegate.cpp`、`ImageNodeEditor/gui/WorkflowNodePainter.cpp`、`ImageNodeEditor/qml/`、`struct.md`、`third_party/THIRD_PARTY_SOURCES.md`

处理方式：
`MainWindow` 继续保留 QAction、文件对话框、执行和保存等业务协调职责，但窗口 chrome 改由 QML 表面承载，并强制迁移到 Dark 主题。`WorkbenchBridge` 增加节点库拖拽、窗口移动/最小化/最大化/关闭请求；`WorkflowCanvas` 接收 `application/x-imagenode-type` 拖放 payload，换算到 scene 坐标后走现有新增节点路径，并在拖拽悬停时绘制落点预览。Qt Nodes 样式、节点 painter、节点内参数控件和全局 QSS 改为 VS Code Dark token；QML 使用项目自绘矢量图标，未引入 VS Code/Microsoft 品牌资产或 Codicons 字体文件。

当前状态：
已解决

后续注意：
本阶段仍保留隐藏的原生菜单/工具栏对象作为 QAction 容器，方便快捷键和命令面板复用；后续若继续清理，应先确认所有 QAction 都已由命令注册表覆盖，再删除旧可视工具栏创建路径。

---

### 2026-05-25 / 原生窗口栏与拖拽崩溃修复阶段

类型：修复

概述：
恢复平台原生窗口栏和窗口按钮，重排 QML 顶部命令栏与左侧 Activity Bar，并修复画布已有节点时从节点库拖入新节点可能崩溃的问题。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/main.cpp`、`ImageNodeEditor/gui/AppIcon.*`、`ImageNodeEditor/gui/MainWindow.cpp`、`ImageNodeEditor/gui/WorkflowCanvas.cpp`、`ImageNodeEditor/qml/WorkbenchTitleBar.qml`、`ImageNodeEditor/qml/WorkbenchActivityBar.qml`、`ImageNodeEditor/qml/WorkbenchSidebar.qml`、`ImageNodeEditor/qml/WorkbenchIcon.qml`、`struct.md`

处理方式：
移除桌面窗口默认 frameless 设置，保留 Qt 原生菜单栏和系统窗口按钮；QML 顶栏改为应用命令栏，左上角显示原创节点图像 workflow 图标，右侧放运行、预览、底部面板和设置等命令。Activity Bar 保持 48px 宽度但放大图标，并增加问题与运行诊断入口。拖拽修复的根因是 drop 回调中新增节点会重建 Qt Nodes scene，而旧 drop preview 图元随后被再次删除；现在 drop 时先读取 payload 和 scene 坐标、清理预览图元，再用延迟回调执行新增节点，`rebuild()` 也会在替换 scene 前清理临时图元。

当前状态：
已解决

后续注意：
`WorkbenchBridge` 中旧的窗口控制 invokable 目前保留但 QML 默认不再调用；后续做 API 清理时可一并移除。

---

### 2026-05-25 / VS Code Dark 贴近化与 Codicons 图标阶段

类型：修改

概述：
继续把工作台视觉向 VS Code Dark 靠拢，去掉窗口内左上角产品图标，统一中文提示与主要界面文案，引入 Codicons 字体子集替换 QML 手绘图标，并为 macOS 增加透明标题栏融合策略。

影响范围：
`CMakeLists.txt`、`ImageNodeEditor/gui/MainWindow.cpp`、`ImageNodeEditor/gui/NativeWindowChrome.*`、`ImageNodeEditor/gui/WorkbenchModels.cpp`、`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/qml/`、`third_party/codicons/`、`struct.md`

处理方式：
QML 顶部命令栏移除产品图标区域，macOS 通过 AppKit 隐藏标题文字并启用透明标题栏，Windows/Linux 保留原生标题栏路径。新增 `WorkbenchTooltip.qml` 统一深色悬停说明框，Activity Bar、顶栏、编辑区头部和节点库提示改为中文。`WorkbenchIcon.qml` 改为加载 vendored `codicon.ttf` 并按内部图标名映射到 Codicons glyph；原生菜单动作设置为不在菜单中显示图标，只保留中文文字和快捷键。

当前状态：
已解决

后续注意：
本轮按中文优先实现，未引入完整语言切换。Codicons 仅作为图标字体使用，并保留许可证与来源说明；不要引入 VS Code/Microsoft 品牌图形。

---

### 2026-05-25 / 设置页、低速缩放与整理画布优化阶段

类型：修改

概述：
继续贴近 VS Code Dark 工作台风格，重写设置页，降低画布滚轮缩放速度，并把原“自动布局”升级为面向用户的“整理画布”功能。

影响范围：
`ImageNodeEditor/gui/MainWindow.*`、`ImageNodeEditor/gui/WorkflowCanvas.*`、`ImageNodeEditor/qml/WorkbenchTitleBar.qml`、`ImageNodeEditor/qml/WorkbenchEditorHeader.qml`、`ImageNodeEditor/qml/WorkbenchSidebar.qml`、`struct.md`

处理方式：
设置页从旧 `QFormLayout` 改为左侧分类导航加右侧内容页，按“常用 / 画布 / 工作台 / 快捷键”组织；新增滚轮缩放速度设置、整理画布入口、命令面板入口、工作台区域开关和当前命令快捷键查询。Qt Nodes 默认滚轮缩放每格约 `1.2x`，现在由 `WorkflowCanvas` 拦截滚轮事件并把步进交给 `MainWindow`，默认每格约 `1.04x`，工具按钮缩放也降为 `1.06x`。整理画布继续使用现有 workflow 图模型和撤销栈，但布局算法改为先校验连线，再按拓扑层级从左到右排列，并对同层节点做输入/输出重心排序，按端口与参数估算节点高度后居中排布。

当前状态：
已解决

后续注意：
快捷键页本轮只提供查询和搜索，不做快捷键重绑定。整理画布采用内置 DAG 分层算法，不引入 Graphviz/Dagre 运行时依赖，后续若需要更强的交叉最小化可在当前算法后继续增加局部交换优化。

---

### 2026-05-25 / 方案库与进度记录阶段

类型：新增

概述：
把 VS Code 的“扩展/源代码管理”侧栏概念改造成更适合图像 workflow 的“方案库”和“进度记录”：方案库提供预设处理流程和用户自定义模板；进度记录提供轻量保存点、恢复和分支式尝试。

影响范围：
`ImageNodeEditor/gui/MainWindow.*`、`ImageNodeEditor/gui/WorkbenchHostWidget.*`、`ImageNodeEditor/gui/WorkbenchModels.*`、`ImageNodeEditor/qml/WorkbenchActivityBar.qml`、`ImageNodeEditor/qml/WorkbenchSidebar.qml`、`ImageNodeEditor/qml/WorkbenchIcon.qml`、`struct.md`

处理方式：
新增 `WorkflowTemplateModel` 和 `WorkflowCheckpointModel` 暴露给 QML。方案库内置“灰度预览、缩放导出、文字叠加预览、双图混合预览”四个预设 workflow，用户也可以把当前 workflow 保存为模板；用户模板以 JSON 快照保存到 `QStandardPaths::AppDataLocation/templates`，元数据保存在 `QSettings`。进度记录同样保存 JSON 快照到 `checkpoints`，每条记录带分支名；恢复保存点和套用模板都会覆盖当前画布，但通过现有 `pushGraphEditCommand` 进入撤销栈。从保存点创建分支会载入该保存点、切换当前轻量分支名，并自动写入一条分支创建记录。

当前状态：
已解决

后续注意：
本轮没有实现完整 Git 语义，不做 merge/rebase/stash，也没有在线模板市场。后续可以在当前模板模型上增加删除、重命名、导入/导出模板包等操作。

---

### 2026-05-25 / VS Code Dark 交互修复与卡顿治理阶段

类型：修改

概述：
修复画布空白右键菜单、QML 悬浮提示被裁剪、设置页按钮和复选框布局拥挤的问题，并把工作流执行与实时预览改为后台线程执行，降低复杂任务时 UI 卡死的概率。

影响范围：
`ImageNodeEditor/gui/MainWindow.*`、`ImageNodeEditor/gui/WorkflowCanvas.*`、`ImageNodeEditor/gui/WorkbenchModels.*`、`ImageNodeEditor/qml/WorkbenchTooltip.qml`

处理方式：
`WorkflowCanvas` 新增空白画布上下文菜单回调，由 `MainWindow` 生成 VS Code Dark 风格菜单，提供快捷添加节点、整理画布、重置视图、区域开关、保存进度和保存模板。`WorkbenchTooltip.qml` 不再直接绘制 `Popup`，改为通过 `WorkbenchBridge` 请求 C++ 全局 tooltip 浮层，避免被多个 `QQuickWidget` surface 裁剪。设置页扩大最小尺寸，右侧内容进入滚动区域，复选框 indicator、按钮高度和底部按钮文案统一调整为中文。`runWorkflow()` 和 `runLivePreview()` 基于 graph/engine 快照在线程中执行，UI 线程只接收节点状态和完成结果；执行期间禁止新增节点、连线、整理、套用模板、恢复进度等会替换图结构的操作。

当前状态：
已解决

后续注意：
本轮没有实现节点级取消执行；关闭窗口时仍会等待后台执行自然结束。后续如果要支持中断，需要给 `ExecutionEngine` 增加取消令牌并传递到节点执行过程。

---

### 2026-05-27 / 退出稳定性、全中文界面、设置页与整理画布优化阶段

类型：修复 / 修改

概述：
按用户六项诉求做稳定性与界面优化：1) 修复退出时 QML 访问悬空对象导致的报错/崩溃；2) 加固关闭与析构路径并加顶层异常兜底；3) 界面进一步全中文化（节点统一显示中文名加序号，引擎日志、状态栏、大纲、快速搜索、下拉选项、文件筛选器、品牌词全部去英文，内部 ID 不变）；4) 删除顶栏右上角重复的设置图标；5) 修复设置页文字/控件重叠并去重含糊选项；6) 整理画布后自动居中并按合适缩放呈现全图。

影响范围：
`ImageNodeEditor/main.cpp`、`ImageNodeEditor/core/NodeLabel.h`（新增）、`CMakeLists.txt`、`ImageNodeEditor/gui/MainWindow.h/.cpp`、`ImageNodeEditor/gui/WorkbenchHostWidget.h/.cpp`、`ImageNodeEditor/gui/WorkbenchModels.cpp`、`ImageNodeEditor/gui/WorkflowNodeDelegate.cpp`、`ImageNodeEditor/workflow/ExecutionEngine.cpp`、`ImageNodeEditor/nodes/BasicNodes.cpp`、`ImageNodeEditor/qml/WorkbenchTitleBar.qml`、`ImageNodeEditor/qml/WorkbenchSidebar.qml`

处理方式：
退出崩溃根因是 `workbenchBridge_` 与各 Model（均 parent 给 `MainWindow`）先于承载它们的 `QQuickWidget` 析构，QML 绑定访问悬空对象。新增 `WorkbenchHostWidget::teardownSurfaces()` 清空所有 QML 表面并断开桥接连接，在 `~MainWindow` 中于 bridge/Model 析构前调用；`closeEvent` 与析构统一递增执行/预览 generation、停止定时器并先等待后台线程；`main` 在 `app.exec()` 外加异常兜底。新增 header-only `formatNodeLabel(displayName, id)` 生成“中文名 #序号”，在大纲、状态栏、参数/删除/导航日志、快速搜索及 `ExecutionEngine` 摘要/日志中替换裸英文 ID（内部 ID 仍英文，JSON 不变）；`WorkflowNodeDelegate` 的 Choice 下拉用值→中文映射显示但回写英文值，文件对话框筛选器改中文。设置页把单一外层 `QScrollArea` 改为每页各自 `QScrollArea`，修正换行提示标签因 `heightForWidth` 无法穿过 `QStackedWidget` 而重叠的问题，并去除多页重复的“整理画布/重置布局”按钮、补全分页说明。整理画布新增 `MainWindow::fitGraphInView()`（基于 `scene_->itemsBoundingRect()` 计算缩放并居中，clamp 0.25–3.0），`autoLayoutWorkflow()` 末尾无条件调用，画布右键“重置视图”也改为适应全图。

当前状态：
已解决（编译通过；CLI `--no-gui` 执行示例工作流成功且日志为中文）。

后续注意：
GUI 交互与退出无报错需在真实显示环境验证：本机以 `QT_QPA_PLATFORM=offscreen` 无法冒烟测试，因为 `NativeWindowChrome::configure` 依赖真实 `NSWindow`，离屏平台下 `winId()` 非 NSView 会在 AppKit 崩溃；该路径在正常带显示运行时有效，未改动。`formatNodeLabel` 的序号取 ID 最后一个下划线之后的部分，依赖现有 ID 命名习惯。

---

### 2026-06-01 / 设置页排版、命令面板外部关闭、窗口缩放与标题栏阶段

类型：修复 / 修改

概述：
按用户三项诉求做界面修复：1) 重排首选项设置页，根治按钮/控件重叠；2) 命令面板（快速访问浮层）支持单击外部自动关闭；3) 补全窗口缩放体验——标题栏窗口控制按钮、双击标题栏最大化、拖动标题栏移动窗口、记住窗口大小/最大化状态，并增强 macOS 自绘标题栏。

影响范围：
`ImageNodeEditor/gui/MainWindow.h/.cpp`、`ImageNodeEditor/gui/WorkbenchHostWidget.h/.cpp`、`ImageNodeEditor/gui/WorkbenchModels.h/.cpp`、`ImageNodeEditor/qml/WorkbenchTitleBar.qml`、`ImageNodeEditor/qml/WorkbenchIcon.qml`（`NativeWindowChrome_mac.mm` 本身未改，只是在 `changeEvent` 中复用其 `configure`）

处理方式：
设置页（`MainWindow::showSettingsDialog`）在 `makePage/makeSection/makeRow` 的容器布局统一加 `QLayout::SetMinimumSize`，并把换行提示标签收敛到 `makeHintLabel`（垂直 `QSizePolicy::Minimum`），使容器最小高度始终容纳换行文字真实高度，配合每页 `QScrollArea` 竖向滚动，彻底消除上一轮残留的按钮重叠；动作按钮统一经 `styleActionButton`（Fixed 尺寸+显式最小高度）放进 `makeButtonRow`（按钮+末尾弹簧）。命令面板：`WorkbenchHostWidget` 覆写 `eventFilter` 并 `qApp->installEventFilter(this)`，浮层可见时监听全局 `MouseButtonPress`，按下点在浮层全局矩形外即调用 `bridge_->activateQuickAccess(-1)`（复用与 Esc 一致的取消路径），不消费事件。窗口：`WorkbenchTitleBar.qml` 右侧新增最小化/最大化-还原/关闭三枚按钮（`visible: Qt.platform.os !== "osx"`，macOS 复用红绿灯），空白区加 `DragHandler`（移动窗口）+`TapHandler`（双击最大化），均复用既有 `requestWindowMove/Maximize/Minimize/Close` invokable；`WorkbenchBridge` 新增只读 `windowMaximized` 属性，`MainWindow::changeEvent` 在 `WindowStateChange` 时同步该状态并重新套用 `NativeWindowChrome::configure`（防止 macOS 最大化/还原后 styleMask 丢失 FullSizeContentView 而重现黑色原生标题条）；`createLayout` 的 `restoreGeometry` 改为失败才回退默认尺寸，确保跨会话记住窗口大小/最大化。`WorkbenchIcon.qml` 补 codicon 窗口控制字形。

当前状态：
编译通过（Qt 6.11，Unix Makefiles）；CLI `--no-gui` 执行示例工作流回归通过；QML 经 `qmllint` 无语法/类型错误。GUI 视觉效果（尤其 macOS「真正隐藏标题栏」是否还残留黑条）需在用户真机显示环境确认——本机 `screencapture` 无显示权限、离屏平台又会因 `NativeWindowChrome` 依赖真实 `NSWindow` 而无法冒烟。

后续注意：
macOS 顶部「黑色标题条」若仍存在，根因多为 Qt 在几何更新时按自身 styleMask 认知重算内容视图、抵消手动设置的 FullSizeContentView；当前以 show/状态变更时重套 chrome 缓解，若仍不彻底需在真机迭代（可能要直接调整内容视图 frame 或重排顶部工具栏层级）。Windows 端本轮保持原生标题栏（已可缩放/最大化），未引入全 frameless 改造。

---

### 2026-06-03 / 设置残影、保存时间线、退出崩溃、节点紧凑化、命令面板关闭、顶栏去重阶段

类型：修复 / 新增 / 修改

概述：
按用户六项诉求继续打磨：1) 设置页消除导航挤叠与半透明残影；2) 新增「保存时间线」（Ctrl+S 自动记录可恢复快照，并入「进度记录」面板）；3) 修复退出时程序异常退出（崩溃）；4) 节点控件紧凑化、提升信息密度；5) 命令面板改为「点击外部即关闭」；6) 右上角去重，顶栏只留 QML 标题栏一处命令按钮。

影响范围：
`ImageNodeEditor/gui/MainWindow.h/.cpp`、`ImageNodeEditor/gui/WorkbenchHostWidget.h/.cpp`、`ImageNodeEditor/gui/WorkbenchModels.h/.cpp`、`ImageNodeEditor/gui/WorkflowNodeDelegate.cpp`、`ImageNodeEditor/gui/WorkflowNodePainter.cpp`、`ImageNodeEditor/qml/WorkbenchSidebar.qml`

处理方式：
退出崩溃：上一轮新增的 `MainWindow::changeEvent` 在 `WindowStateChange` 时调 `NativeWindowChrome::configure`（销毁期访问原生 NSWindow）及 `qApp` 全局事件过滤器是主要风险面。本轮新增 `shuttingDown_` 标志（`closeEvent`/析构置位），`changeEvent` 去掉 chrome 重套只保留 `setWindowMaximized` 并加 `!shuttingDown_ && isVisible()` 守卫，`showEvent` 同样守卫；并给内嵌终端 `TerminalPanel` 补析构 `kill` 子进程，消除退出期 `QProcess` 警告。实测：真机 `quit` 事件正常退出 code=0、无新崩溃报告、stderr 干净。
命令面板：移除全局事件过滤器，改为自包含透明遮罩层 `ClickScrim`（`WorkbenchHostWidget` 匿名子类），`showQuickAccess` 时铺满并置于面板之下，点面板外即 `activateQuickAccess(-1)` 关闭；同时消除全局过滤器的退出崩溃面。
设置页：导航 `QListWidget` 改用显式 `QListWidgetItem::setSizeHint`+`uniformItemSizes`+`spacing`，杜绝项挤叠；page/section 加 `WA_StyledBackground` 让实色背景完整绘制，消除半透明残影。
保存时间线：复用 `WorkflowCheckpointModel` 新增第二实例 `workflowTimelineModel_`（QML 上下文属性 `workflowTimelineModel`，经 `WorkbenchHostWidget` 构造注入）；`saveWorkflow` 成功后 `recordSaveTimeline()` 存快照到 `timeline/` 并记 QSettings 组 `workflowTimeline`（上限 30，超出连文件删除）；`refreshWorkflowTimelineModel` 用「今天/昨天/MM-dd HH:mm」生成简洁标题；`WorkbenchBridge::restoreTimeline` → `MainWindow::restoreTimelineEntry` 载入快照并入撤销栈；侧栏「进度记录」面板顶部新增「时间线」小节并提供「恢复」。
节点紧凑化：`WorkflowNodeDelegate::buildParameterPanel` 边距 9/7→6/4、间距 8/6→6/3、label 宽 54→40 右对齐、面板最小宽 196→150；控件 min-height 24→22、padding 收紧、「...」按钮宽 34→26；`WorkflowNodePainter` 头部 38→32。
顶栏去重：移除 `headerToolbar_` 右侧 预览/底部面板/设置 按钮与 `workbookToolbar_` 的 执行 按钮，命令统一走 QML 标题栏（设置仍在左侧活动栏/命令面板）。

当前状态：
编译通过（Qt 6.11）；CLI `--no-gui` 回归 exit 0；`qmllint` 无错；退出崩溃已真机验证为干净退出。设置/节点/顶栏/时间线/命令面板等视觉与交互需用户真机确认（本机 `screencapture` 与合成按键均被系统权限拦截，无法自动截图/录屏）。

后续注意：
时间线快照与手动 checkpoint 各自独立存储（`timeline` vs `workflowCheckpoints` 组），互不影响。`headerToolbar_` 仍保留居中文档标题与前进/后退，与 QML 标题栏存在轻度重复，如需进一步精简可后续再议。

---

### 2026-06-03 / 未保存退出崩溃根因修复、设置标题栏深色、节点拖拽创建、编辑器头部去重阶段

类型：修复 / 修改

概述：
按用户 5 项反馈处理 4 项（终端 req5 经用户确认本轮不做）：1) 彻底修复「未保存改动时退出崩溃」（上一轮只修了一处，仍崩）；2) 设置对话框白色原生标题栏改深色；3) 节点库单击不再加节点、仅拖拽到画布才创建；4) 删除编辑器头部与顶部标题栏重复的 执行/预览/面板 等按钮。全部经 computer-use 真机复现验证通过。

影响范围：
`ImageNodeEditor/gui/MainWindow.cpp`、`ImageNodeEditor/gui/WorkflowCanvas.cpp`、`ImageNodeEditor/gui/NativeWindowChrome.h/.cpp/_mac.mm`、`ImageNodeEditor/main.cpp`、`ImageNodeEditor/qml/WorkbenchSidebar.qml`、`ImageNodeEditor/qml/WorkbenchEditorHeader.qml`

处理方式：
退出崩溃用真机 `.ips` backtrace 定位到**两处**「析构期信号回调访问半析构对象」：① `~QUndoStack::clear()` 发 `indexChanged` → 构造里连接的 `updateWindowTitle` lambda → `syncCurrentWorkbookPage→graphForPersistence`，访问已析构成员；修法：`~MainWindow` 开头 `undoStack_->disconnect()` + `updateWindowTitle` 顶部 `if(shuttingDown_) return`。② `~WorkflowCanvas` 默认析构时 `scene_`(unique_ptr) 删除 NodeGraphicsObject 触发 `selectionChanged` → `updateExpandedNodes` 访问正在析构的 scene（`~unique_ptr` 不像 `reset()` 先置空，故 `if(!scene_)` 守卫失效）；修法：把 `~WorkflowCanvas()=default` 改为显式析构，先 `scene_->disconnect()` 再让成员析构。两处都只在「画布有节点 / 撤销栈非空」即未保存时才触发，故空画布测不出。设置标题栏：`NativeWindowChrome` 新增 `applyGlobalDarkAppearance()`（macOS `NSApp.appearance=DarkAqua`），`main.cpp` 启动时调用一次，所有窗口/对话框/红绿灯统一深色。节点拖拽：`WorkbenchSidebar.qml` 节点库 delegate 去掉 `onClicked/onDoubleClicked` 的 `createNode`，单击只设 `currentIndex` 高亮，拖拽（已有 `startNodeDrag`+画布 `dropEvent`）才创建。编辑器头部：`WorkbenchEditorHeader.qml` 删除 add/layout/preview/panel/执行，仅留 文档标题 + 前进/后退面包屑。

当前状态：
编译通过（Qt 6.11）；CLI `--no-gui` 回归 exit 0；computer-use 真机验证：拖节点制造未保存改动→关闭→不保存→**干净退出、无新崩溃报告**；设置标题栏深色、节点单击不加/拖拽加、编辑器头部无重复，均已截图确认。

后续注意：
「析构期信号回调」是本项目反复出现的崩溃类型（已修 undoStack、scene 两处），后续新增「构造里 connect 到访问成员状态的 lambda」时，记得在析构开头断开或加 `shuttingDown_` 守卫。req5 原生终端（PTY+VT100）经用户同意暂不做，留待单独一轮。

---

### 2026-06-03 / 侧栏面板错位、右键英文菜单残留、宏节点端口阶段

类型：修复 / 修改

概述：
1) 修正活动栏「方案库/进度记录/工作流」点开后显示内容错位；2) 清除右键时中文菜单背后重叠的 QtNodes 自带英文菜单；3) 让宏节点按内部节点的自由端口生成输入/输出，可像普通节点一样连线，封装/编辑后端口随之更新。

影响范围：
`ImageNodeEditor/qml/WorkbenchSidebar.qml`、`ImageNodeEditor/gui/WorkflowCanvas.cpp`、`ImageNodeEditor/gui/MainWindow.cpp`

处理方式：
req1：`WorkbenchSidebar.qml` 的 `StackLayout` 子面板物理顺序是 0 节点库/1 方案库/2 进度记录/3 工作流/4 搜索/5 问题/6 诊断，但 `currentIndex` 表达式把 workflow→1、templates→2、history→3，三者错位（真机点击复现：点方案库出进度记录、点进度记录出工作流）。改 `currentIndex` 为 templates→1、history→2、workflow→3 与物理顺序对齐。req2：vendored `QtNodes::GraphicsView::contextMenuEvent` 右键节点/组时总会弹自带英文 `createStdMenu`(Copy/Cut)/`createGroupMenu`，与 App 经 `nodeContextMenu` 信号弹出的中文菜单重叠；且这两个方法非 virtual 无法重写。改为在 App 的 `WorkflowGraphicsView` 重写 `contextMenuEvent`，只调用祖父类 `QGraphicsView::contextMenuEvent`（让节点项发 `nodeContextMenu` 信号→中文节点菜单），空白处再调 `nodeScene()->createSceneMenu`（已是中文场景菜单），不走 QtNodes 那套英文菜单；不改 third_party。req3：`MacroNode` 本就有 `inputPorts/outputPorts`（由 mappings 生成），但 `encapsulateSelectionAsMacro` 只按跨选区边界连线建端口，孤立/未外连的内部端口不暴露、`leaveMacroNode` 也不重算。新增 `computeMacroPortMappings(subgraph,…)`：取子图「自由端口」（输入无内部来源、输出无内部去向），顺序按内部节点位置（上→下、左→右），macroPort 用稳定名 `in:/out:节点:端口`。封装时用它生成端口，并对跨界连线的内部端口用 `ensureMacroPort` 补端口（输出被内部同时消费时）以保留既有外部连线；`leaveMacroNode` 编辑子图后重算端口并裁掉父图中引用已消失宏端口的连线。旧 JSON 里 `in1/out1` 的映射照常加载（不在加载时重算），向后兼容。

当前状态：
编译通过（Qt 6.11）；CLI `--no-gui` 回归 exit 0；`qmllint` 无错。三项的 GUI 真机验证因用户锁屏离开暂未完成（computer-use 无法在锁屏下操作），待解锁后用 computer-use 确认：方案库/进度记录/工作流面板内容与按钮一致、右键只剩中文菜单、封装/进入/返回宏节点端口随内部节点变化且可连线。

后续注意：
宏端口稳定名 `in:/out:node:port` 与旧 `in1/out1` 并存；新封装用新名，旧存档载入用存档里的名字，互不影响。若以后要给宏端口做「重命名/隐藏某端口」，需在 `computeMacroPortMappings` 之外加用户级覆盖层。

---

### 2026-06-03 / 界面美化：冷静中性+柔化蓝 的圆角化/精致化阶段

类型：修改

概述：
按用户「更有设计感、类 Claude/Codex、圆角稍多、克制用色、精致排版、微动效」的诉求，对整套深色 UI 做成体系的视觉刷新（保持冷静中性深底，把电光蓝换成柔化蓝）。引入设计 token（颜色/圆角/字号/动效/阴影）并统一应用到 全局 QSS、QML 各面、画布节点、各对话框/菜单/提示。

影响范围：
`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/gui/MainWindow.cpp`、`ImageNodeEditor/gui/WorkbenchHostWidget.cpp`、`ImageNodeEditor/gui/WorkflowCanvas.cpp`、`ImageNodeEditor/gui/WorkflowNodeDelegate.cpp`、`ImageNodeEditor/gui/WorkflowNodePainter.cpp`、`ImageNodeEditor/qml/Workbench{TitleBar,ActivityBar,Sidebar,EditorHeader,StatusBar,QuickAccess}.qml`

处理方式：
设计 token：底色 base #1b1c1e / panel #212327 / elevated #26282d / input #2c2f34；hairline 边 #2e2f33；柔化蓝点缀 #6ea0e0（替代 #3794ff）、选中染色 #303a47（替代硬 #094771/#04395e）；圆角 sm6/md8/lg12；字号基准 11→12，section 标题由「11px Bold 全大写」改 12px DemiBold 句首式+letterSpacing+muted；动效 120–180ms OutCubic；阴影改柔（blur28/offset8）。
AppTheme.cpp：把「先写浅色再正则替换成深色」（且其实早已 return、是死代码）整段删除，`styleSheet()` 直接按 token 产出深色 QSS，全量加 border-radius（按钮/输入/菜单/标签页/滚动条/提示/zoom 浮层），`appFont` 基准 12，`makeShadow` 调柔，`NodeMetrics.cornerRadius` 0→12，`colors()` 与 `apply()` 调色板换 token。
QML：复用组件（CommandButton/ActivityButton/HeaderButton/SidebarHeader/SearchField）集中加圆角+token 配色+`Behavior on color/strokeColor` 渐变；活动栏选中改圆角小竖条+inset 圆角高亮；状态栏弃用刺眼 #007acc 改深色+hairline 顶线+muted 文字；侧栏各 delegate 颜色经 replace-all 统一换 token；命令面板加 radius12、唤起时淡入+轻微放大（由 `quickAccessRequested` 信号触发）、搜索改无框下划线、结果行圆角+渐变。
节点画布：`WorkflowNodePainter` 用 `drawRoundedRect`（半径 12）画阴影/主体/状态条，头部裁剪到圆角内绘制并在顶部留一抹克制类别色，去掉左侧整条工业色条，边框 hairline、选中柔化蓝外发光；`WorkflowCanvas` 的 NodeStyle/ConnectionStyle 字色/连接点/选中色换 token、连接点直径 8→9，网格更稀更淡。
其它面：节点内联参数、设置对话框卡片/导航、画布右键菜单、悬浮提示、底部标签页 全部圆角化+token 配色；命令面板容器 QFrame 与 tooltip 顶层窗口设透明背景/`WA_TranslucentBackground` 让圆角真正显示。

当前状态：
编译通过（Qt 6.11）；CLI `--no-gui` 回归 exit 0；改动 QML 经 `qmllint` 无新错（仅既有 layout 警告）。视觉效果本轮未能用 computer-use 截图确认（本机 ScreenCaptureKit 截图报 SCContentFilter 失败，疑似屏幕录制权限/会话问题），但程序已在用户屏幕上运行可直接查看。

后续注意：
QSS 不支持过渡动画，Widgets 部分（设置框/终端/停靠面板/内联参数）只有圆角+配色、hover 仍瞬切；微动效集中在 QML（按钮/命令框 hover 渐变、命令面板登场）。未做：按钮按下缩放、tooltip 淡入（plan 中标为可选）。如某处颜色/圆角想再调，token 集中在 `AppTheme.cpp` 顶部与各 QML 复用组件，便于微调。

---

### 2026-06-04 / 节点拖拽圆角、节点统一大小+异形、画布去黑条+面包屑+多画布标签阶段

类型：修复 / 修改 / 重构

概述：
画布区 5 项：① 拖动节点圆角毛边；② 节点统一大小、按类别用不同外形+颜色区分；③ 画布顶部黑条；④ 画布上方宏层级面包屑导航；⑤ VS Code 风画布标签条 + 「+」多画布。

影响范围：
`third_party/QtNodes/src/NodeGraphicsObject.cpp`、`ImageNodeEditor/gui/WorkflowNodePainter.cpp`、`ImageNodeEditor/gui/WorkflowNodeDelegate.cpp`、`ImageNodeEditor/gui/MainWindow.h/.cpp`、`ImageNodeEditor/gui/WorkbenchHostWidget.h/.cpp`、`ImageNodeEditor/gui/AppTheme.cpp`

处理方式：
req1：节点项原 `setCacheMode(DeviceCoordinateCache)` 把节点缓存成矩形像素图，拖动时圆角/阴影露矩形毛边；改 vendored 第 39 行为 `NoCache`（注释说明），每帧抗锯齿绘制、轮廓干净。req2：`WorkflowNodeDelegate::buildParameterPanel` 把面板 `setMinimumWidth(150)` 改 `setFixedWidth(184)`+`setMinimumHeight(34)`，所有节点宽度统一；`WorkflowNodePainter` 新增 `categoryShapePath(category, rect)` 按类别画异形轮廓（输入输出=胶囊、几何变换=圆角矩形、色彩处理=小圆角、滤波处理=切角八边形、合成处理=横向六边形），阴影/主体/头部裁剪/选中外发光都改用该 `QPainterPath`，类别色继续作头部色带，做到「形状+颜色」区分；端口仍在左右中点、命中用矩形 boundingRect（异形角落略可点，可接受）。req3/4/5：把画布列顶部重构为 C++ 容器——`viewContainer` 改为竖直 `editorColumn[画布标签条 tabStrip, 面包屑 breadcrumbBar_, 画布 canvasInner]`，作为 editor 传给 host；`WorkbenchHostWidget` 删除原 `editorHeaderSurface_`（那个 QML 编辑器头层就是画布顶黑条来源），黑条消失。标签条复用既有工作簿逻辑（`workbookTabs_`/`newWorkbookAction_`/`addWorkbookPage/switchWorkbookPage/closeWorkbookPage/refreshWorkbookTabs`）从隐藏的 `workbookToolbar_` 迁到画布上方，末尾「+」`canvasNewButton` 新建画布；面包屑用 `graphStack_` 生成可点路径（主画布 › 宏A › …），点某段 `navigateToDepth()` 连续 `leaveMacroNode` 回退，`updateBreadcrumb()` 并入 `updateNavigationActions()` 统一刷新。`AppTheme` QSS 补 `#canvasTabStrip/#breadcrumbBar/QTabBar#workbookTabs/#canvasNewButton/#breadcrumb*` 的圆角+token 样式。

当前状态：
编译通过（Qt 6.11）；CLI `--no-gui` 回归 exit 0；GUI 启动稳定无崩溃（新布局构造正常）。视觉/交互需用户真机确认（computer-use 本轮中途断开，无法自动截图）：拖节点无矩形毛边、节点统一大小且异形区分、画布顶无黑条、标签页「+」可多画布、面包屑可回退。

后续注意：
`WorkbenchEditorHeader.qml` 已不再被引用（仍留在 qrc，无害），后续可从资源清单移除。`workbookToolbar_` 不再创建（成员保留为 nullptr，相关引用处已有 `if` 守卫）。节点异形仅改自绘 painter，未改 `NodeGraphicsObject::shape()`，故点击命中仍按矩形 boundingRect。

---

### 2026-06-05 / 主题系统（深/浅可切换）、字体放大跟随缩放、顶栏色块与拖拽预测框阶段

类型：修复 / 新增 / 重构

概述：
按用户四项诉求：1) 顶栏画布标签条与「+」之间的空白色块、文件添加按钮风格不一致、节点库节点项不统一；2) 从节点库拖拽到画布的「落点预测框」要彻底删除；3) 界面放大时字体不跟随且整体偏小、看不清；4) 设置中新增浅色主题（背景 #F8F8F6、文字 #0A0A0A、绿色为主强调、暗黄为次强调）。核心是把原本「强制深色 + 三处写死十六进制」的配色，重构为单一 palette 数据源，并桥接到 QML。

影响范围：
`ImageNodeEditor/gui/AppTheme.h/.cpp`、`ImageNodeEditor/gui/WorkbenchModels.h/.cpp`（新增 `WorkbenchTheme`）、`ImageNodeEditor/gui/WorkbenchHostWidget.h/.cpp`、`ImageNodeEditor/gui/MainWindow.h/.cpp`、`ImageNodeEditor/gui/WorkflowCanvas.cpp`、`ImageNodeEditor/gui/WorkflowNodePainter.cpp`、`ImageNodeEditor/gui/WorkflowNodeDelegate.cpp`、`ImageNodeEditor/qml/Workbench{Sidebar,TitleBar,ActivityBar,StatusBar,QuickAccess}.qml`

处理方式：
主题核心：`AppTheme` 新增 `Palette`（语义 token）+ `darkPalette()`/`lightPalette()`/`palette()`，`ThemePreference` 加 `Light`，`colors()`/`styleSheet()`/`apply()` 全部从活动 palette 取色（QSS 用 `@token@` 占位 + 替换）；`setThemePreference/isDarkTheme/themePreferenceName` 真正生效。深色 token 沿用原值降低回归。
QML 桥：新增 `WorkbenchTheme`（每 token 为 `Q_PROPERTY(QColor)` + `double scale`，`refresh()`/`setScale()` 发信号），由 `WorkbenchHostWidget` 注册为上下文属性 `theme`；各 QML 表面颜色改绑 `theme.*`，字号改 `Math.round(基准 * theme.scale)` 并整体上调基准——既解决「太小」又让 QML 字体跟随界面缩放（此前 QML 字号写死、完全不随缩放）。
画布/节点：`applyQtNodesDarkStyle`→`applyQtNodesStyle()` 去 static 守卫、JSON 从 palette 生成并在 `rebuild()` 重新应用；`WorkflowNodePainter`/`WorkflowNodeDelegate` 配色改读 palette。**删除拖拽落点框**：去掉 `WorkflowGraphicsView` 的 `dropPreview_`/`updateDropPreview()`，drag 事件只接受不再画矩形，drop 新增节点逻辑不变。
MainWindow：启动读 `mainWindow/theme` 恢复主题；设置页「外观」新增主题下拉（深/浅），QSS、底部标签条、右键菜单、tooltip、设置对话框、图片放大窗等硬编码样式改 palette 驱动；`applyUiScale()` 作为统一刷新枢纽补充 `workbenchTheme_->setScale/refresh`、底部标签条重设样式，主题切换复用它。顶栏：取消 `workbookTabs_` 强制 min-width（消除空白色块），新建画布按钮改用简洁「+」线性图标（`lineIcon("plus")`，随主题/缩放重绘）；节点库 QML delegate 改为统一圆角矩形卡片。`appFont` 基准 12→13、节点标题/标签 15/13→16/14。

当前状态：
编译通过（Qt 6.11）；CLI `--no-gui` 回归 exit 0；改动 QML 经 `qmllint` 无新错（仅既有 unqualified-access 与 layout-positioning 警告）；GUI 默认深色启动稳定、stderr 干净、无 QML 绑定报错。浅色与深色用同一套 `WorkbenchTheme` getter，仅取值不同，故深色无错即覆盖浅色路径。

后续注意：
QML 字号跟随缩放依赖「绑定经函数读 `theme.scale` 的依赖追踪」，已在 Qt6 验证可重绑。已存在的问题日志项颜色在创建时设定，主题切换后旧条目不重染（下次执行重建即恢复）。真机视觉仍建议确认：浅色整体观感、顶栏无色块、节点库圆角卡片、拖拽无落点框、放大后各处文字清晰。

---

### 2026-06-05 / Chrome 风标签栏、拖拽残影、浅色原生窗口栏、工程结构整洁化阶段

类型：修复 / 修改 / 重构

概述：
承接浅色主题后续四项：1) 顶部画布标签栏做成 Chrome 风（自绘梯形、激活标签与画布融合）；2) 彻底删除从节点库拖拽时跟随光标的深色节点预览块；3) 浅色模式下 macOS 顶部黑色原生标题栏改为浅色原生标题栏（保留红绿灯，随主题切换）；4) 适度工程整洁：移除仓库内被跟踪的废弃构建产物与废弃目录、拆分超大 `MainWindow.cpp`、对齐 `struct.md` 文档（main.cpp 保持在项目同名目录不动）。

影响范围：
`ImageNodeEditor/gui/CanvasTabBar.{h,cpp}`（新增）、`ImageNodeEditor/gui/MiniMapWidget.h`/`PreviewWidgets.h`/`TerminalPanel.h`（新增，从 MainWindow 抽出）、`ImageNodeEditor/gui/AppIcon.{h,cpp}`（接收 `lineIcon`）、`ImageNodeEditor/gui/AppTheme.cpp`、`ImageNodeEditor/gui/MainWindow.{h,cpp}`、`ImageNodeEditor/gui/WorkbenchModels.cpp`、`ImageNodeEditor/gui/NativeWindowChrome.{h,cpp,_mac.mm}`、`ImageNodeEditor/main.cpp`、`CMakeLists.txt`、`.gitignore`、`struct.md`；删除 `build-prototypes/`（含取消 Git 跟踪）与 `nodeeditor/`。

处理方式：
任务1：新增 `CanvasTabBar : QTabBar`，`paintEvent` 自绘——激活标签为顶部圆角、底部外扩梯形并以 base 内容色填充、盖住条底 hairline 与下方画布融为一体；非激活标签扁平、相邻间细分隔线、hover 浅染；strip 底色改 panel 让激活标签凸显。`MainWindow` 用它替换原 `QTabBar`，并删除 `AppTheme` 里 `QTabBar#workbookTabs::tab` 的 QSS。
任务2：`WorkbenchModels::startNodeDrag` 删除原本写死的深色 `QPixmap`（`#2a2d2e` 等）与 `setPixmap/setHotSpot`，改用 1x1 全透明拖拽图并去掉 text mime，杜绝深色残影与平台默认文本拖影。
任务3：`NativeWindowChrome_mac.mm::configure` 改读 `AppTheme::isDarkTheme()`/`palette().base`，浅色用 `NSAppearanceNameAqua`+浅色背景、深色用 `DarkAqua`+深色背景；`applyGlobalDarkAppearance`→`applyGlobalAppearance`（按主题设 `NSApp.appearance`）。`main.cpp` 在调用前先按保存的主题 `setThemePreference`，保证浅色启动即浅色原生栏；设置页切换主题后在 `applySettings` 守卫调用 `applyGlobalAppearance`+`configure(this)` 即时生效。
任务4：`git rm -r --cached build-prototypes` 并加入 `.gitignore`，从磁盘删除 `build-prototypes/`、`nodeeditor/` 两个废弃目录。`MainWindow.cpp` 4944→4412 行：`lineIcon()` 移入 `AppIcon`（调用点统一改 `AppIcon::lineIcon`）；`ImagePopupWindow`+`PreviewLabel`→`PreviewWidgets.h`、`MiniMapWidget`→`MiniMapWidget.h`、`TerminalPanel`→`TerminalPanel.h`（均无 Q_OBJECT，header-only 搬运，行为零改变），新文件登记进 `CMakeLists`。`struct.md` 修正 core/nodes/processing/util/gui 的实际文件清单与「仅深色」过期描述。

当前状态：
编译通过（Qt 6.11，每步增量构建均通过）；CLI `--no-gui` 回归 exit 0；GUI 默认深色启动稳定、stderr 干净、无 QML/主题报错。

后续注意：
header-only 内部控件（MiniMap/Preview/Terminal）无信号槽，故无需 moc、无 `.cpp`；若日后给它们加信号槽需补 Q_OBJECT 并拆出 `.cpp`。Chrome 标签底部外扩弧靠盖住条底 hairline 实现「与内容融合」，依赖 strip(panel)/内容(base) 两色有别——若日后让二者同色需另加分隔。任务1/3 的真机视觉仍建议确认：标签栏 Chrome 观感、浅色顶栏为浅色原生、深浅切换即时且不崩溃。

---

### 2026-06-05 / 标签栏放大美化、命令面板黑尖角、picdeal 全功能命令行阶段

类型：修复 / 新增 / 重构

概述：
三项：1) 画布标签栏太小不美观 → 放大并精致化；2) 命令面板圆角浮层四角露黑尖角 → 修复；3) 新增 ffmpeg 式 picdeal 命令行（pipe/build/run/validate/nodes + 类 git 的 save/log/restore 历史回溯），整体能力接近 GUI，且与 GUI 共用同一套保存历史。

影响范围：
`ImageNodeEditor/main.cpp`、`ImageNodeEditor/gui/CanvasTabBar.cpp`、`ImageNodeEditor/gui/MainWindow.cpp`、`ImageNodeEditor/util/PathUtils.cpp`、`ImageNodeEditor/workflow/WorkflowHistory.{h,cpp}`（新增）、`ImageNodeEditor/app/CommandLineApp.{h,cpp}`（新增）、`CMakeLists.txt`、`struct.md`。

处理方式：
任务1：`CanvasTabBar` tabSizeHint 高度 `fm.height()+14→+24`、宽度内边距加大、min 宽 120→150；paintEvent 顶部圆角 10→12、底部外扩弧 9→11、top inset 上移，标签文字略大且激活标签半粗体。
任务2：根因是没有带 alpha 的默认 `QSurfaceFormat`，`QQuickWidget` 透明清屏被清成黑，圆角浮层四角露黑。`main.cpp` 创建 `QApplication` 前 `QSurfaceFormat::setDefaultFormat`（alphaBufferSize=8）。
任务3：新增 `WorkflowHistory`（GUI 无关）封装保存历史的 dataDir/list/save/load，严格沿用既有 QSettings 组（workflowTimeline/workflowCheckpoints）+ AppData 快照格式；GUI 的 `recordSaveTimeline/refreshWorkflowTimelineModel/saveWorkflowCheckpoint/refreshWorkflowCheckpointModel` 与 `workflowDataDir` 改为调用它（单一事实源、互通已实测）。新增 `CommandLineApp` 子命令调度：`pipe` 把 `-i/--op/-o` 构建为线性图并执行，`build --save` 存为 workflow.json，`run/validate/nodes`，`save/log/restore` 走 `WorkflowHistory`（默认 checkpoints，`--timeline` 看自动保存）。`main.cpp` 识别首参为子命令时进入命令行；`CMakeLists` POST_BUILD 生成 `build/picdeal` 便捷可执行（macOS/Linux 符号链接、Windows 复制 .exe）。顺带修复 `PathUtils::resolveAgainstFile` 用 `QDir::cleanPath` 先消 `..`，避免 `/tmp/../Users/...` 这类因符号链接解析错位（GUI 亦受益）。

当前状态：
编译通过（Qt 6.11）。命令行实测：`picdeal pipe -i in.ppm --grayscale --blur 3 --resize 320x240 -o out.png` 出图；`build → run` 往返成功；`--blend 第二图 opacity=` 成功；`nodes/validate/run/--no-gui` 正常；`save → log → restore` 成功，且 `log` 能看到 GUI 既有保存点（GUI↔CLI 历史互通已验证）。GUI 启动稳定、stderr 干净。

后续注意：
命令行用 `QGuiApplication` + 默认 `offscreen` 平台（不弹窗、无显示也能渲染）：文字叠加等节点要字体/QPainter，`QCoreApplication` 渲染文字会 abort；若用户已设 `QT_QPA_PLATFORM` 则尊重其设置。
`pipe` 仅支持单输入线性管线；分支/汇合复杂图仍用 workflow.json（build 也只构建线性）。CLI 把输入/输出路径转绝对再存，配合 cleanPath 修复保证 build/--save 后异目录 run 仍能找到文件。GUI 的「模板」与 restore/branch 仍用原有 `file/<id>` 读取（与历史模块同格式，未重写，降低对撤销栈/图替换路径的回归风险）。`picdeal` 是同一二进制的便捷链接，GUI 目标名仍为 ImageNodeEditor。真机视觉仍建议确认：标签栏更大更精致、命令面板四角无黑尖角（深/浅色）。
