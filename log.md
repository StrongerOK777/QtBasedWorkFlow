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
