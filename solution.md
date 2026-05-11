# solution.md：Qt + C++ 节点式图像处理工作流工具实施方案

## 0. 本文件用途

本文件是给后续开发者和编码工具使用的开发说明。目标不是一次性生成完整项目，而是指导它按模块、按阶段、按可测试标准实现一个跨平台的 Qt + C++ 图像处理节点工作流桌面原型。

本项目需要完成一个类似 Scratch / Node-RED / 蓝图编辑器的图像处理工作流工具：用户可以在 Qt 图形界面中添加节点、移动节点、连接端口、修改参数、预览节点输出、一键执行整个流程，并且能把工作流保存为 JSON，下次加载后重新执行。它面向的是图像处理流程搭建场景：把读图、裁切、缩放、调色、滤波、文字叠加、预览、导出等操作拆成节点，让用户像搭积木一样组合流程，而不是每次手工操作或临时写脚本。

这个工具的目标不是只做一条固定的图片处理流水线，而是做一个可扩展的桌面原型。基础版本必须稳定支持常见图像处理节点和工作流执行；后续可以继续扩展蒙版、通道、拼图、风格化、评分等更复杂节点。

重点要求：

- 使用 C++17 + Qt。
- 支持 macOS 开发、Windows 重新编译运行。
- 不要求 macOS 编译出的程序直接复制到 Windows 运行。
- 要保证源码仓库在 Windows 下重新配置 CMake 后能够生成 `.exe`。
- 基础功能优先于复杂特效。
- 程序遇到错误必须提示用户，不能崩溃。
- 尽量避免依赖平台专属库。
- 外部库不是禁止使用，但必须可选化、隔离化，不能让基础功能依赖难以配置的第三方库。

---

## 1. 总体目标

实现一个桌面端节点式图像处理工具：

```text
读入图片节点
    ↓
裁切节点
    ↓
调色节点
    ↓
模糊节点
    ↓
预览节点 / 导出节点
```

工作流可以是线性的，也可以包含分支和汇合。例如同一张图片可以分成两路：一路灰度化，另一路增强对比度，之后再通过合并、混合或拼接节点生成最终结果。

用户在界面中搭建工作流，程序负责：

1. 检查节点端口类型是否匹配。
2. 检查工作流是否为有向无环图。
3. 检查输入文件、参数、输出路径是否有效。
4. 按依赖顺序执行节点。
5. 显示结果或导出结果。
6. 保存和加载工作流 JSON。
7. 可选支持命令行模式执行 JSON 工作流。

最低功能必须覆盖：

```text
读入图片
显示 / 预览图片
导出图片
裁切、缩放、旋转 / 翻转等变换
灰度、亮度 / 对比度等色彩处理
模糊等基础滤波
至少 8 种节点类型
```

---

## 2. 推荐技术栈

### 2.1 必选

```text
语言：C++17
GUI 框架：Qt 6，优先使用 Qt Widgets
构建系统：CMake
JSON：Qt 自带 QJsonDocument / QJsonObject / QJsonArray
图像基础处理：Qt 自带 QImage / QPainter / QColor / QTransform
节点画布：QGraphicsView / QGraphicsScene / QGraphicsItem
```

### 2.2 不建议作为基础依赖的内容

基础版本不要强依赖以下内容：

```text
OpenCV
ImageMagick
Python
FFmpeg
平台专属 API
网络外部 API
Shell 命令
```

原因：这些库在 macOS 和 Windows 上配置方式不同，会显著增加跨平台编译风险。

### 2.3 如果必须使用 OpenCV

如果后续想加入 OpenCV：

- OpenCV 只能作为可选后端。
- 基础节点必须不依赖 OpenCV 也能运行。
- 必须将 OpenCV 相关代码放入独立模块，例如 `ImageNodeEditor/backend/opencv/`。
- CMake 中用 `find_package(OpenCV QUIET)`，找不到时禁用相关节点，而不是让整个项目编译失败。
- UI 中如果 OpenCV 后端不可用，应隐藏或禁用对应节点，并显示说明。

示例 CMake 思路：

```cmake
find_package(OpenCV QUIET)

if(OpenCV_FOUND)
    target_compile_definitions(ImageNodeEditor PRIVATE HAS_OPENCV=1)
    target_link_libraries(ImageNodeEditor PRIVATE ${OpenCV_LIBS})
else()
    target_compile_definitions(ImageNodeEditor PRIVATE HAS_OPENCV=0)
endif()
```

---

## 3. 跨平台总原则

### 3.1 正确认知

正确目标：

```text
源码仓库复制到 Windows
→ Windows 安装 Qt / CMake / MSVC 或 MinGW
→ 重新配置 CMake
→ 重新编译
→ 生成 Windows .exe
```

错误目标：

```text
macOS 编译出 .app
→ 复制到 Windows
→ 直接运行
```

`.app` 和 `.exe` 是不同平台的二进制格式，不能通用。

### 3.2 必须避免

禁止在代码中出现：

```cpp
system("open xxx");
system("start xxx");
system("rm -rf xxx");
system("mkdir xxx");
#include <unistd.h>
#include <sys/stat.h>
#include <CoreFoundation/CoreFoundation.h>
#include <windows.h>  // 除非隔离在平台专属模块
```

打开文件或目录统一使用：

```cpp
QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
```

文件路径统一使用：

```cpp
QDir
QFileInfo
QStandardPaths
QCoreApplication::applicationDirPath()
```

不要直接拼接平台路径：

```cpp
// 不推荐
QString path = base + "/" + name;

// 推荐
QString path = QDir(base).filePath(name);
```

### 3.3 路径规则

必须支持：

```text
macOS: /Users/name/Desktop/a.png
Windows: C:/Users/name/Desktop/a.png
中文路径：C:/Users/张三/Desktop/测试图片.png
相对路径：assets/input.png
```

注意事项：

- Qt 中统一使用 `/` 即可，Qt 会在底层适配平台。
- 不要硬编码 `/Users/...` 或 `C:\Users\...`。
- JSON 中推荐保存相对路径。
- 用户选择文件时使用 `QFileDialog`。
- 内部处理路径时使用 `QString`，不要随意转成 `std::string` 后交给 C 风格文件 API。
- 读写文件优先使用 `QFile`。

### 3.4 资源文件规则

`resources.qrc` 不是 Qt 项目的必需文件。基础版本可以先不用自定义资源文件，只使用文字按钮或 Qt 标准图标；当项目需要内置图标、示例图片、示例 workflow 或默认配置时，再加入 Qt Resource System。

如果使用 Qt Resource System，推荐结构：

```text
resources.qrc
resources/icons/
resources/samples/
```

代码访问方式：

```cpp
QIcon icon(":/icons/open.png");
```

图标来源建议：

- 基础版优先使用文字按钮，或者使用 Qt 自带的 `QStyle::standardIcon()`。
- 如果需要更统一的视觉效果，再准备 SVG / PNG 图标放入 `resources/icons/`。
- 自定义图标可以来自自己绘制、开源图标库或课程允许的素材，但要注意授权。

不要把基础功能建立在运行目录中的零散图标文件上，否则换平台后容易丢失资源。自定义资源只是改善界面，不应该影响工作流核心功能。

---

## 4. 推荐项目结构

```text
ImageNodeEditor/
  CMakeLists.txt
  README.md
  solution.md
  struct.md
  log.md
  agent.md

  ImageNodeEditor/
    main.cpp

    app/
      AppController.h
      AppController.cpp
      CommandLineRunner.h
      CommandLineRunner.cpp

    core/
      PortType.h
      NodeId.h
      Edge.h
      NodeParameter.h
      NodeData.h
      Result.h

    nodes/
      ImageNode.h
      ImageNode.cpp
      NodeFactory.h
      NodeFactory.cpp
      ImageInputNode.h
      ImageInputNode.cpp
      ImageOutputNode.h
      ImageOutputNode.cpp
      PreviewNode.h
      PreviewNode.cpp
      CropNode.h
      CropNode.cpp
      ResizeNode.h
      ResizeNode.cpp
      GrayscaleNode.h
      GrayscaleNode.cpp
      BrightnessContrastNode.h
      BrightnessContrastNode.cpp
      GaussianBlurNode.h
      GaussianBlurNode.cpp
      RotateFlipNode.h
      RotateFlipNode.cpp
      TextOverlayNode.h
      TextOverlayNode.cpp
      BlendNode.h
      BlendNode.cpp
      ImageMergeNode.h
      ImageMergeNode.cpp

    processing/
      CropProcessor.h
      CropProcessor.cpp
      ResizeProcessor.h
      ResizeProcessor.cpp
      ColorProcessor.h
      ColorProcessor.cpp
      BlurProcessor.h
      BlurProcessor.cpp
      ComposeProcessor.h
      ComposeProcessor.cpp

    workflow/
      WorkflowGraph.h
      WorkflowGraph.cpp
      WorkflowSerializer.h
      WorkflowSerializer.cpp
      WorkflowValidator.h
      WorkflowValidator.cpp
      ExecutionEngine.h
      ExecutionEngine.cpp

    gui/
      MainWindow.h
      MainWindow.cpp
      NodeScene.h
      NodeScene.cpp
      NodeItem.h
      NodeItem.cpp
      PortItem.h
      PortItem.cpp
      EdgeItem.h
      EdgeItem.cpp
      PropertyPanel.h
      PropertyPanel.cpp
      PreviewPanel.h
      PreviewPanel.cpp
      NodePalette.h
      NodePalette.cpp
      LogPanel.h
      LogPanel.cpp

    util/
      PathUtils.h
      PathUtils.cpp
      ImageUtils.h
      ImageUtils.cpp
      JsonUtils.h
      JsonUtils.cpp

    resources/
      icons/
      samples/
      workflows/

    resources.qrc  # 可选，需要内置资源时再加入

  tests/
    test_workflow_json.cpp
    test_graph_validation.cpp
    test_nodes.cpp
```

结构要求：

- `gui/` 只负责窗口、画布、交互和参数控件。
- `nodes/` 只负责节点类型、端口定义、参数保存和调用图像处理逻辑。
- `processing/` 放纯图像算法函数，不依赖 `MainWindow`、`QGraphicsScene` 等界面对象。
- `workflow/` 负责图结构、JSON 读写、验证、拓扑排序和执行。
- `util/` 放路径、JSON、图片格式等通用辅助函数。
- `ImageNodeEditor/main.cpp` 放在与项目同名的子目录中，便于后续迁移到要求这种目录组织的 Visual Studio 工程。
- `resources.qrc` 是可选资源清单，不是基础功能必需文件。
- GUI 模式和命令行模式若都实现，必须共用 `workflow/`、`nodes/`、`processing/`，不能各写一套执行逻辑。
- `solution.md` 是总体设计和实施方案，说明项目目标、技术路线、核心规则和验收标准。
- `struct.md` 是项目结构索引，详细说明每个目录、每类文件、主要类的职责，方便开发者快速定位应该修改的位置。
- `log.md` 是开发和运行问题记录文件，用于简要记录程序运行中出现的问题、项目中做过的重要修改、当前遗留的问题和后续处理结论。记录要言简意赅，不需要逐行描述代码改动，但要能让后续开发者知道问题发生在哪里、如何处理、是否已解决。
- `agent.md` 是给辅助编码工具使用的最终编码指导文件，说明工程风格、排错方式、修改边界和开发纪律。

`log.md` 建议按条目记录：

```text
日期 / 阶段
问题或修改概述
影响范围
处理方式
当前状态
后续注意事项
```

示例：

```text
2026-05-11 / 核心模型阶段
问题：加载 JSON 时未知节点类型会导致空指针。
影响范围：WorkflowSerializer、NodeFactory。
处理方式：NodeFactory 返回 Result，并在 UI / CLI 中显示错误。
状态：已解决。
后续：新增节点时必须注册到 NodeFactory。
```

---

## 5. CMake 跨平台要求

`CMakeLists.txt` 必须明确 C++17，并关闭编译器扩展。

示例：

```cmake
cmake_minimum_required(VERSION 3.16)

project(ImageNodeEditor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

qt_add_executable(ImageNodeEditor
    ImageNodeEditor/main.cpp

    ImageNodeEditor/core/Result.h
    ImageNodeEditor/core/PortType.h
    ImageNodeEditor/core/NodeData.h

    ImageNodeEditor/nodes/ImageNode.h
    ImageNodeEditor/nodes/ImageNode.cpp
    ImageNodeEditor/nodes/NodeFactory.h
    ImageNodeEditor/nodes/NodeFactory.cpp

    ImageNodeEditor/workflow/WorkflowGraph.h
    ImageNodeEditor/workflow/WorkflowGraph.cpp
    ImageNodeEditor/workflow/WorkflowSerializer.h
    ImageNodeEditor/workflow/WorkflowSerializer.cpp
    ImageNodeEditor/workflow/WorkflowValidator.h
    ImageNodeEditor/workflow/WorkflowValidator.cpp
    ImageNodeEditor/workflow/ExecutionEngine.h
    ImageNodeEditor/workflow/ExecutionEngine.cpp

    ImageNodeEditor/gui/MainWindow.h
    ImageNodeEditor/gui/MainWindow.cpp

    # ImageNodeEditor/resources.qrc  # 可选，需要内置资源时启用
)

target_link_libraries(ImageNodeEditor PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

if(MSVC)
    target_compile_options(ImageNodeEditor PRIVATE /permissive- /W4)
else()
    target_compile_options(ImageNodeEditor PRIVATE -Wall -Wextra -Wpedantic)
endif()
```

要求：

- 不把 `build/` 纳入源码仓库。
- 不把 `CMakeCache.txt` 纳入源码仓库。
- 不把 `.app`、`.exe`、`.dll`、`.dylib`、`.o` 等构建产物纳入源码仓库。
- Windows 端必须重新配置 CMake。
- 如果使用 Qt Creator，打开 `CMakeLists.txt` 即可。

---

## 6. GUI 功能设计

GUI 要让用户在不修改代码的情况下完成完整流程：

```text
从节点列表添加节点
→ 在画布上移动节点并连接端口
→ 选中节点后修改参数
→ 执行整个工作流
→ 在预览区查看某个节点或最终结果
→ 保存 workflow.json
→ 重新打开 workflow.json 后继续执行或修改
```

主窗口建议分为五个区域：

```text
┌──────────────────────────────────────────────┐
│ 菜单栏 / 工具栏：新建、打开、保存、执行、导出 │
├──────────────┬───────────────────┬───────────┤
│ 节点列表      │ 节点画布            │ 属性面板   │
│ NodePalette  │ QGraphicsView      │ 参数编辑   │
├──────────────┴───────────────────┴───────────┤
│ 预览区域 / 日志区域                            │
└──────────────────────────────────────────────┘
```

### 6.1 节点列表

至少提供以下节点：

1. ImageInputNode：读入图片。
2. ImageOutputNode：导出图片。
3. PreviewNode：预览图片。
4. CropNode：裁切。
5. ResizeNode：缩放。
6. GrayscaleNode：灰度化。
7. BrightnessContrastNode：亮度 / 对比度。
8. GaussianBlurNode：模糊。
9. RotateFlipNode：旋转 / 翻转。
10. TextOverlayNode：文字叠加。
11. BlendNode：双图混合，可用于分支汇合。
12. ImageMergeNode：多图拼接或网格合并，可用于九宫格、对比图等场景。

至少实现 8 种，建议实现 10 种以上，方便保底。基础节点必须覆盖读图、显示、导出、图像变换、滤波和色彩处理；`BlendNode`、`ImageMergeNode`、通道拆分 / 合并、蒙版混合、拼图、风格化、图像评分等可以作为扩展节点逐步加入。

节点列表可以按类别分组：

```text
输入输出：读入图片、导出图片、预览图片
几何变换：裁切、缩放、旋转 / 翻转
色彩处理：灰度化、亮度 / 对比度、通道处理
滤波处理：模糊、锐化、边缘增强
合成处理：文字叠加、图片混合、图片拼接
扩展节点：蒙版、风格化、评分、外部算法
```

### 6.2 节点画布

使用：

```text
QGraphicsView
QGraphicsScene
QGraphicsItem
```

支持：

- 从节点列表添加节点到画布。
- 拖动节点，并保存节点在画布上的位置。
- 删除节点，同时删除与该节点相关的所有连线。
- 删除连线。
- 点击或拖拽端口开始连线。
- 连接时检查端口方向和类型。
- 连接非法时不创建边，并提示原因。
- 选中节点后，在属性面板显示参数。
- 允许用户多次清空、重建、保存、加载并执行不同工作流。
- 节点执行后可显示状态：未执行、执行中、成功、失败。
- 用户点击某个节点时，预览区域可以显示该节点上一次执行后的输出图像。
- 如果当前节点没有可预览输出，预览区域显示明确说明，而不是空白崩溃。

### 6.3 属性面板

属性面板根据节点类型动态显示参数。

例如：

```text
CropNode:
  x
  y
  width
  height

GaussianBlurNode:
  radius

BrightnessContrastNode:
  brightness
  contrast
```

参数修改后：

- 只更新当前节点参数。
- 标记工作流为 dirty。
- 不应该立即导致程序崩溃。
- 对明显非法值进行 UI 层限制，例如半径不能为负数，裁切宽高必须大于 0。
- 即使 UI 已限制参数，节点执行时仍要再次验证，因为 JSON 文件可能被手动修改。
- 文件路径类参数使用 `QFileDialog` 选择，不能要求用户手工输入固定绝对路径。

---

## 7. 核心数据模型设计

核心模型必须独立于 GUI。`MainWindow` 和 `NodeScene` 只能通过 `WorkflowGraph` 修改节点、端口和连线，不能自己保存一套重复的业务图结构。

每个节点必须明确声明：

```text
节点类型 typeName
显示名称 displayName
输入端口数量、端口名称、端口类型
输出端口数量、端口名称、端口类型
参数名称、类型、默认值、合法范围
执行逻辑
```

端口名称必须稳定，因为 JSON 会用端口名称保存连线。不要用 UI 显示文字作为唯一标识，建议端口内部名使用英文，例如 `input`、`image`、`mask`、`output`。

### 7.1 端口类型

```cpp
enum class PortType {
    ImageRGBA,
    ImageGray,
    Number,
    Text,
    Mask,
    ImageList
};
```

### 7.2 端口方向

```cpp
enum class PortDirection {
    Input,
    Output
};
```

建议端口信息结构：

```cpp
struct PortInfo {
    QString name;
    QString displayName;
    PortDirection direction;
    PortType type;
    bool required = true;
    bool allowMultipleConnections = false;
};
```

默认规则：

- 普通输入端口只能连接一条边。
- 输出端口可以连接到多个下游输入端口，用于分支。
- 只有明确设置 `allowMultipleConnections` 的输入端口才能接收多条边，例如多图合并节点。
- 同一节点内部不能从输出端口连回自己的输入端口，除非后续专门设计并验证为安全行为；基础版直接禁止。

### 7.3 节点基类

节点必须使用继承和多态，不能靠一个巨大 `if-else` 判断所有节点类型。

节点创建使用 `NodeFactory` 注册机制。`WorkflowSerializer` 读取 JSON 中的 `type` 后，通过工厂创建节点；工厂找不到类型时返回错误，不允许程序崩溃。

建议接口：

```cpp
class ImageNode {
public:
    virtual ~ImageNode() = default;

    virtual QString typeName() const = 0;
    virtual QString displayName() const = 0;

    virtual QVector<PortInfo> inputPorts() const = 0;
    virtual QVector<PortInfo> outputPorts() const = 0;

    virtual QJsonObject saveParams() const = 0;
    virtual Result<void> loadParams(const QJsonObject& obj) = 0;

    virtual Result<QMap<QString, NodeData>> execute(
        const QMap<QString, NodeData>& inputs
    ) = 0;
};
```

### 7.4 NodeData

节点之间不要直接传裸指针。建议用值对象或隐式共享类型。

```cpp
struct NodeData {
    PortType type;
    QVariant value;
};
```

对于图片：

```cpp
QImage image = data.value.value<QImage>();
```

注意：

- QImage 是隐式共享类型，适合跨节点传递。
- 修改图片前必要时使用 copy，避免意外共享修改。

### 7.5 Result 类型

所有核心逻辑不要直接崩溃，不要随便 `throw` 到 UI 层。

建议设计统一结果类型：

```cpp
template <typename T>
class Result {
public:
    static Result<T> ok(T value);
    static Result<T> fail(QString message);

    bool isOk() const;
    const QString& error() const;
    const T& value() const;
};
```

对于无返回值：

```cpp
using VoidResult = Result<std::monostate>;
```

也可以简单实现：

```cpp
struct Status {
    bool ok;
    QString message;
};
```

最低要求：所有节点执行失败时都返回错误信息，而不是崩溃。

---

## 8. 工作流图设计

工作流图是有向图，节点表示图像处理步骤，边表示数据从上游节点的输出端口流向下游节点的输入端口。基础版本必须支持一条输出边连接多个下游节点，从而形成分支；如果实现汇合节点，则汇合节点通过多个输入端口接收来自不同分支的数据。

用户在 GUI 中看到和操作的是节点框、端口圆点和连线；程序真正执行时使用的是 `WorkflowGraph` 中的数据。每次用户从一个端口拖线到另一个端口，UI 层都要把这次操作转换成一条 `Edge`：

```text
fromNode: 上游节点 id
fromPort: 上游输出端口名
toNode: 下游节点 id
toPort: 下游输入端口名
```

`EdgeItem` 只负责在画布上画出连线，`Edge` 才是保存、加载和执行工作流时使用的真实连接关系。

典型分支与汇合示例：

```text
ImageInput
  ├─ Grayscale ─────┐
  └─ Brightness ────┤
                    ↓
                 Blend / Merge
                    ↓
                 Preview / Output
```

### 8.1 保存内容

JSON 保存的是工作流配方，不是处理后的图片本身。

必须保存：

```text
节点 id
节点 type
节点位置
节点参数
连线 from node / from port / to node / to port
文件格式版本号
```

不保存：

```text
节点运行时缓存
预览图像二进制内容
用户机器上的临时绝对路径
UI 选中状态
```

如果工作流文件和图片在同一项目目录下，图片路径优先保存为相对路径；加载时以 workflow 文件所在目录作为相对路径基准。

外部图片导入规则：

- 用户必须能通过 `ImageInputNode` 从外部选择图片文件。
- 选择图片使用 `QFileDialog`，不要要求用户手工输入固定路径。
- 选择后将图片路径写入该节点参数 `path`。
- 保存 workflow JSON 时保存这个 `path`，优先保存相对路径。
- 为了便于复用和迁移，GUI 可以提供“复制到项目 assets 目录”选项，把外部图片复制到当前 workflow 旁边的 `assets/` 目录，再在 JSON 中保存 `assets/xxx.png`。
- 如果用户不复制图片，只保存原路径，则下次打开 workflow 时需要检查原图片是否仍存在；不存在时给出明确提示。

用户保存的实际工作流项目可以形成这样的运行时目录，这个目录不等同于源码目录：

```text
MyWorkflowProject/
  workflow.json
  assets/
    input_orange.png
    logo.png
  output/
    result.png
```

### 8.2 JSON 示例

```json
{
  "formatVersion": 1,
  "projectName": "ImageNodeEditorDemo",
  "nodes": [
    {
      "id": "node_1",
      "type": "ImageInput",
      "position": { "x": 100, "y": 120 },
      "params": {
        "path": "assets/input.png"
      }
    },
    {
      "id": "node_2",
      "type": "Crop",
      "position": { "x": 360, "y": 120 },
      "params": {
        "x": 20,
        "y": 20,
        "width": 300,
        "height": 300
      }
    },
    {
      "id": "node_3",
      "type": "GaussianBlur",
      "position": { "x": 620, "y": 120 },
      "params": {
        "radius": 3
      }
    },
    {
      "id": "node_4",
      "type": "ImageOutput",
      "position": { "x": 880, "y": 120 },
      "params": {
        "path": "output/result.png"
      }
    }
  ],
  "edges": [
    {
      "fromNode": "node_1",
      "fromPort": "image",
      "toNode": "node_2",
      "toPort": "input"
    },
    {
      "fromNode": "node_2",
      "fromPort": "output",
      "toNode": "node_3",
      "toPort": "input"
    },
    {
      "fromNode": "node_3",
      "fromPort": "output",
      "toNode": "node_4",
      "toPort": "input"
    }
  ]
}
```

### 8.3 版本号说明

`formatVersion` 是 JSON 文件格式版本号，不是 Git 版本管理。

不要实现复杂 Git 功能。可选实现：

- 最近打开文件。
- 自动保存。
- 另存为。
- 简单撤销 / 重做。
- 快照保存。

---

## 9. 工作流验证规则

执行前必须验证工作流。

验证失败必须返回清晰错误信息，并在 GUI 中弹窗或写入日志面板；命令行模式中输出到 stderr。任何验证错误都不能导致空指针访问、数组越界或程序崩溃。

### 9.1 基础验证

必须检查：

- JSON 是否能解析。
- `formatVersion` 是否支持。
- 所有节点 id 是否唯一。
- 所有 edge 指向的 node 是否存在。
- fromPort 是否存在。
- toPort 是否存在。
- fromPort 必须是输出端口。
- toPort 必须是输入端口。
- 端口类型必须兼容。
- 必填参数是否存在。
- 输入文件是否存在。
- 输出目录是否可写。
- 图是否为 DAG。
- 不允许形成环。
- 每个节点的必需输入是否都有上游数据来源。
- 普通输入端口是否被多条边连接。
- 多输入汇合节点的输入数量是否满足节点要求。
- 分支输出是否仍然保持类型一致。
- 参数类型是否和节点声明一致，例如数字参数不能读取成字符串后直接使用。

### 9.2 DAG 检查

使用拓扑排序。

如果拓扑排序后节点数量少于图中节点数量，说明存在环。

伪代码：

```text
compute indegree for every node
push nodes with indegree == 0 into queue
while queue not empty:
    pop node
    append to execution order
    for each outgoing edge:
        decrease indegree
        if indegree becomes 0:
            push target node

if execution order size != node count:
    fail("工作流中存在环，无法执行")
```

### 9.3 连接检查

用户拖线时立即检查：

```text
输出端口 → 输入端口：允许继续检查类型
输入端口 → 输入端口：禁止
输出端口 → 输出端口：禁止
类型不匹配：禁止
同一输入端口已有连接：默认禁止，除非该端口显式允许多输入
产生环：禁止
```

---

## 10. 执行引擎设计

执行引擎只依赖核心模型和节点类，不依赖 GUI。GUI 模式点击“执行”和命令行模式执行 JSON 时，都应该调用同一个 `ExecutionEngine`，这样验证规则、节点行为和错误处理保持一致。

### 10.1 执行流程

```text
用户点击执行
→ 保存当前 UI 参数到 WorkflowGraph
→ WorkflowValidator 检查
→ ExecutionEngine 拓扑排序
→ 按顺序执行节点
→ 每个节点读取上游输出
→ 当前节点生成输出
→ 缓存输出给下游节点
→ 遇到错误停止执行并显示错误
```

对于分支流程，一个节点的输出可以被多个下游节点读取；执行引擎只需缓存一次上游输出。对于汇合节点，执行前要从所有已连接输入端口收集对应的上游数据，缺少必需输入时直接失败。

### 10.2 执行数据缓存

建议：

```cpp
QMap<NodeId, QMap<QString, NodeData>> nodeOutputs;
```

每个节点执行完成后，将输出保存起来。

缓存键必须包含节点 id 和端口名，不能只按节点 id 保存单一图像，因为后续可能出现一个节点多个输出端口，例如通道拆分节点输出 `r`、`g`、`b` 三路图像。

### 10.3 节点执行错误处理

任何节点失败都不能崩溃。

例如：

```text
ImageInputNode:
  文件不存在 → fail("输入图片不存在：xxx")
  QImage 加载失败 → fail("无法读取图片，可能格式不支持或文件损坏")

CropNode:
  没有输入图像 → fail("裁切节点缺少输入图像")
  裁切区域越界 → fail("裁切区域超出图像范围")
  width <= 0 或 height <= 0 → fail("裁切宽高必须大于 0")

GaussianBlurNode:
  radius < 0 → fail("模糊半径不能为负数")
  输入图像为空 → fail("模糊节点收到空图像")

ImageOutputNode:
  输出路径为空 → fail("导出路径不能为空")
  目录不存在且创建失败 → fail("无法创建输出目录")
  保存失败 → fail("图片保存失败")
```

### 10.4 防止 UI 卡死

基础版本可以同步执行，但至少要：

- 执行期间禁用“执行”按钮。
- 显示日志。
- 执行结束后恢复按钮。
- 出错后恢复按钮。

增强版本可以把执行放入 `QThread` 或 `QtConcurrent`，但注意：

- GUI 对象只能在主线程更新。
- 工作线程只处理纯数据和 QImage。
- 通过 signal / slot 把结果传回主线程。

---

## 11. 节点实现建议

### 11.1 ImageInputNode

参数：

```text
path: QString
copyToProjectAssets: bool，可选
```

输出：

```text
image: ImageRGBA
```

执行：

```cpp
QImage img(path);
if (img.isNull()) return fail(...);
return image converted to Format_RGBA8888;
```

注意：

- 路径优先按 workflow 文件所在目录解析相对路径。
- 不能写死绝对路径。
- UI 中必须提供“选择图片”入口，使用 `QFileDialog` 从外部导入图片。
- 选择图片后，节点参数面板显示图片路径，并允许重新选择。
- 如果启用 `copyToProjectAssets`，程序将图片复制到 workflow 所在目录的 `assets/` 子目录，后续 JSON 保存相对路径，便于把整个项目目录移动到其他机器。
- 如果复制失败，保留原路径并提示用户，不能崩溃。

### 11.2 ImageOutputNode

输入：

```text
input: ImageRGBA
```

参数：

```text
path: QString
```

执行：

- 检查输入图像是否为空。
- 检查输出目录。
- 调用 `QImage::save()`。
- 返回成功或失败。

### 11.3 PreviewNode

输入：

```text
input: ImageRGBA
```

输出：

```text
output: ImageRGBA
```

作用：

- 不改变图像。
- 将图像传递给 UI 预览面板。
- 命令行模式中可以跳过预览行为。

### 11.4 CropNode

参数：

```text
x, y, width, height
```

执行：

- 检查区域合法。
- 用 `QImage::copy(QRect(...))`。

### 11.5 ResizeNode

参数：

```text
width, height, keepAspectRatio
```

执行：

- 用 `QImage::scaled()`。
- 宽高必须大于 0。
- 大图缩放时注意内存。

### 11.6 GrayscaleNode

执行：

```cpp
img.convertToFormat(QImage::Format_Grayscale8);
```

输出类型可以是：

```text
ImageGray
```

如果后续节点只接受 `ImageRGBA`，需要提供兼容规则或自动转换节点。

### 11.7 BrightnessContrastNode

参数：

```text
brightness: [-100, 100]
contrast: [-100, 100]
```

执行：

- 遍历像素。
- 注意截断到 `[0, 255]`。
- 不要访问越界像素。
- 大图处理可能较慢，基础版可以接受，但要避免崩溃。

### 11.8 GaussianBlurNode

如果不用 OpenCV，可以实现简化版：

- 半径为 0 时直接返回原图。
- 用两次一维均值模糊近似。
- 或者实现简单 box blur，节点名称可叫 `BlurNode`，避免算法名不准确。

参数：

```text
radius: [0, 20]
```

必须限制最大半径，避免计算量过大。

### 11.9 RotateFlipNode

参数：

```text
operation: rotate90 / rotate180 / rotate270 / flipHorizontal / flipVertical
```

使用：

```cpp
QTransform transform;
```

### 11.10 TextOverlayNode

参数：

```text
text
x
y
fontSize
color
```

使用：

```cpp
QPainter painter(&image);
```

注意：

- 文本为空时允许直接返回原图。
- 字体大小要限制合理范围。
- QPainter 必须正确 end，或者使用局部对象自动析构。

### 11.11 BlendNode

作用：

```text
将两张图片按透明度混合，用于工作流分支汇合。
```

输入：

```text
base: ImageRGBA
overlay: ImageRGBA
```

输出：

```text
output: ImageRGBA
```

参数：

```text
opacity: [0.0, 1.0]
mode: normal
```

执行：

- 检查两张输入图像是否都存在。
- 基础版要求两张图尺寸一致；尺寸不一致时返回错误，或明确按 `base` 尺寸缩放 `overlay`。
- 使用 `QPainter` 设置透明度绘制。
- `opacity` 越界时返回错误，不要静默使用非法值。

### 11.12 ImageMergeNode

作用：

```text
把多张图片拼成横向、纵向或网格布局，用于对比图、九宫格、批量预览。
```

输入：

```text
image1: ImageRGBA
image2: ImageRGBA
image3...image9: ImageRGBA，可选
```

输出：

```text
output: ImageRGBA
```

参数：

```text
layout: horizontal / vertical / grid
columns: int
backgroundColor: QColor
cellWidth, cellHeight: int，可选
```

执行：

- 至少需要两张有效输入图像。
- `columns` 必须大于 0。
- 基础版可以把每张图缩放到同一单元格后绘制到新 `QImage`。
- 输出尺寸必须有上限，避免拼接超大图导致内存暴涨。

### 11.13 可选扩展节点

扩展节点不要影响基础功能编译和运行。可以逐步加入：

```text
ChannelSplitNode：输入彩色图，输出 R / G / B 三个灰度图
ChannelMergeNode：输入 R / G / B 三个灰度图，输出彩色图
MaskBlendNode：输入前景、背景、蒙版，输出合成图
GridSplitNode：输入一张图，输出 ImageList 或多个裁切结果
SharpenNode：锐化
EdgeDetectNode：边缘检测
StylizeNode：素描 / 油画 / 卡通风格
ImageScoreNode：输出亮度、对比度、主色调等简单评分数据
```

如果新增 `ImageList`、`Mask`、`Number` 等数据类型节点，必须同步更新端口兼容矩阵、JSON 示例、验证规则和测试用例。

---

## 12. 端口兼容性矩阵

基础兼容规则：

| 输出类型 | 输入类型 | 是否允许 | 说明 |
|---|---:|---:|---|
| ImageRGBA | ImageRGBA | 是 | 彩色图直接传递 |
| ImageGray | ImageGray | 是 | 灰度图直接传递 |
| ImageGray | ImageRGBA | 可选 | 可自动转成 RGB |
| ImageRGBA | ImageGray | 可选 | 可自动灰度化，但建议用显式 Grayscale 节点 |
| Number | Number | 是 | 数值参数 |
| Text | Text | 是 | 文本参数 |
| Mask | Mask | 是 | 蒙版 |
| ImageList | ImageList | 是 | 图片列表 |
| 其他组合 | 其他组合 | 否 | 默认禁止 |

建议基础版严格一些：不自动隐式转换，要求用户添加转换节点。这样更容易解释和调试。

分支不改变端口类型规则：一个 `ImageRGBA` 输出端口可以同时连接多个 `ImageRGBA` 输入端口。汇合节点也不放宽类型规则，它只是拥有多个输入端口，每个输入端口仍然按矩阵检查。

如果后续实现自动转换，必须把转换行为写清楚。例如 `ImageGray` 自动接入 `ImageRGBA` 时，是复制到 RGB 三通道；`ImageRGBA` 自动接入 `ImageGray` 时，是按固定公式灰度化。基础版不建议隐式转换。

---

## 13. 防崩溃设计清单

本项目的错误处理目标是：参数错误、文件缺失、端口类型不匹配、JSON 错误、环形依赖、图片过大或保存失败时，用户都能看到清楚原因，程序继续保持可用。

不能只在 UI 层做限制。任何节点从 JSON 加载参数后，执行前都必须再次校验，因为用户可能手动编辑 workflow 文件。

### 13.1 文件读取

必须处理：

- 文件不存在。
- 文件路径为空。
- 文件扩展名不支持。
- 文件损坏。
- 中文路径。
- 相对路径解析失败。
- 没有读取权限。

解决方案：

```text
使用 QFileInfo 检查存在性
使用 QImage::isNull 检查加载结果
错误返回 Result，不抛给 UI 崩溃
```

### 13.2 文件写入

必须处理：

- 输出路径为空。
- 目录不存在。
- 目录创建失败。
- 没有写权限。
- 文件正在被占用。
- 保存格式不支持。

解决方案：

```text
QFileInfo 获取目录
QDir().mkpath(...)
QImage::save 返回 false 时提示
```

### 13.3 JSON 读取

必须处理：

- JSON 文件不存在。
- JSON 语法错误。
- 缺少 nodes / edges。
- 节点 type 未注册。
- 参数类型错误。
- 端口名称错误。
- formatVersion 不支持。

解决方案：

```text
QJsonParseError 获取具体错误
每个字段读取前检查 contains 和类型
未知节点类型不能崩溃，应提示：未知节点类型 xxx
```

### 13.4 图结构

必须处理：

- 节点 id 重复。
- 边指向不存在的节点。
- 端口不存在。
- 端口方向错误。
- 端口类型不兼容。
- 图中存在环。
- 某节点缺少必需输入。
- 某输入端口被多条边连接。

解决方案：

```text
WorkflowValidator 独立检查
执行前强制验证
UI 连线时提前验证
```

### 13.5 图像处理参数

必须处理：

- 裁切区域越界。
- 宽高为 0 或负数。
- 模糊半径过大。
- 缩放尺寸过大。
- 亮度 / 对比度超出范围。
- 文本节点字体大小非法。
- 空图像输入。

解决方案：

```text
属性面板限制输入范围
节点执行时再次验证
不要只依赖 UI，因为 JSON 可以手动修改
```

### 13.6 内存与性能

必须处理：

- 超大图片导致内存暴涨。
- 节点重复执行导致缓存过多。
- 死循环执行。
- 过大的 blur radius 导致长时间卡死。

解决方案：

```text
限制最大图片尺寸，例如宽高不超过 8000 或总像素不超过 50MP
限制 blur radius
每次执行前清空上一次缓存
DAG 验证防止环
执行期间禁用执行按钮
```

### 13.7 指针和对象生命周期

必须避免：

```text
裸 owning pointer
QGraphicsItem 删除后仍被 EdgeItem 引用
QObject 没有 parent 导致泄漏
重复 delete
悬空指针
```

建议：

- 业务模型使用 `std::unique_ptr` 管理节点。
- Qt UI 对象使用 QObject parent 机制。
- QGraphicsScene 删除节点前，先删除相关 EdgeItem。
- UI 层只保存 nodeId，不直接长期保存裸业务节点指针。
- 通过 WorkflowGraph 查询模型对象。

### 13.8 异常边界

建议核心执行层使用 `Result` 返回错误。最外层可以兜底：

```cpp
try {
    auto result = engine.run(workflow);
    if (!result.ok) showError(result.message);
} catch (const std::exception& e) {
    showError(QString("内部错误：") + e.what());
} catch (...) {
    showError("未知内部错误");
}
```

不要把异常作为常规控制流。

---

## 14. 命令行模式设计

支持：

```bash
ImageNodeEditor --workflow workflow.json --no-gui
```

行为：

```text
读取 workflow.json
验证工作流
执行工作流
输出成功或失败信息
退出程序
```

返回码：

```text
0：成功
1：参数错误
2：JSON 读取失败
3：工作流验证失败
4：执行失败
```

实现建议：

```cpp
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();

    QCommandLineOption workflowOption("workflow", "Workflow JSON file.", "file");
    QCommandLineOption noGuiOption("no-gui", "Run without GUI.");

    parser.addOption(workflowOption);
    parser.addOption(noGuiOption);
    parser.process(app);

    if (parser.isSet(noGuiOption)) {
        CommandLineRunner runner;
        return runner.run(parser.value(workflowOption));
    }

    MainWindow w;
    w.show();
    return app.exec();
}
```

说明：

- 即使 no-gui，也可以用 `QApplication`，这样能复用部分 Qt GUI 类型和图片插件。
- 命令行模式不能弹窗，错误要输出到 stderr。
- GUI 模式错误用 QMessageBox 或日志面板显示。

---

## 15. Windows 部署注意事项

Windows 编译后生成的 `.exe` 不一定能在没有 Qt 环境的电脑上运行，因为缺少 Qt DLL 和插件。

部署时使用：

```bash
windeployqt ImageNodeEditor.exe
```

部署包中应该包含：

```text
ImageNodeEditor.exe
Qt6Core.dll
Qt6Gui.dll
Qt6Widgets.dll
platforms/qwindows.dll
imageformats/...
styles/...
```

如果程序读取外部示例工作流和图片，还需要包含：

```text
workflows/
assets/
```

如果资源已放入 `.qrc`，则不需要单独复制图标。

---

## 16. macOS 部署注意事项

macOS 编译后通常是 `.app` 包。

部署时可使用：

```bash
macdeployqt ImageNodeEditor.app
```

注意：

- `.app` 是应用程序包，本质是目录。
- `open ImageNodeEditor.app` 是用系统默认方式启动它。
- 不要在程序内部依赖 `open` 命令。
- 跨平台打开文件用 `QDesktopServices::openUrl`。

---

## 17. 开发阶段顺序

### 阶段 1：项目骨架

目标：

- CMake 项目能在 macOS 编译。
- CMake 项目能在 Windows 编译。
- 主窗口能启动。
- 有基本菜单栏和空画布。

交付：

```text
CMakeLists.txt
main.cpp
MainWindow
空 NodeScene
```

验收：

```text
macOS 能运行
Windows 能重新编译生成 .exe
没有平台专属代码
```

### 阶段 2：核心模型

目标：

- 实现 Node / Port / Edge / WorkflowGraph。
- 实现 NodeFactory。
- 实现基础 JSON 保存 / 加载。
- 不做复杂 GUI。

验收：

```text
手写 demo workflow.json 能被读取
读取后能重建节点和边
保存后 JSON 结构稳定
错误 JSON 不崩溃
```

### 阶段 3：基础节点

目标：

实现至少 8 个节点：

```text
ImageInput
ImageOutput
Preview
Crop
Resize
Grayscale
BrightnessContrast
Blur
RotateFlip
TextOverlay
```

验收：

```text
每个节点有参数验证
每个节点空输入不崩溃
每个节点失败返回清晰错误
```

### 阶段 4：执行引擎

目标：

- 实现工作流验证。
- 实现 DAG 拓扑排序。
- 实现按顺序执行。
- 实现输出缓存。
- 实现错误日志。

验收：

```text
线性流程能执行
分支流程能执行
汇合流程能执行，若实现了合并节点
环形流程被拒绝
类型不匹配被拒绝
```

### 阶段 5：GUI 节点编辑器

目标：

- 节点列表。
- 节点画布。
- 拖动节点。
- 端口连线。
- 删除节点和边。
- 属性面板。
- 预览面板。
- 执行按钮。
- 保存 / 打开工作流。

验收：

```text
用户能不用改代码搭建流程
用户能保存 workflow.json
用户能重新打开 workflow.json
重新打开后能执行
```

### 阶段 6：命令行模式

目标：

```bash
ImageNodeEditor --workflow workflow.json --no-gui
```

验收：

```text
不启动窗口
执行成功返回 0
执行失败返回非 0
命令行输出明确错误
```

### 阶段 7：跨平台验收和部署

目标：

- macOS 编译通过。
- Windows 编译通过。
- Windows 用 windeployqt 部署。
- 检查中文路径。
- 检查相对路径。
- 检查 JSON 回放。

---

## 18. 测试用例设计

### 18.1 正常用例

1. 读图 → 预览。
2. 读图 → 裁切 → 导出。
3. 读图 → 灰度 → 模糊 → 导出。
4. 读图 → 缩放 → 文字叠加 → 导出。
5. 读图 → 分支：一路灰度，一路调亮度，各自预览。
6. 读图 → 分支：一路灰度，一路调亮度 → BlendNode 汇合 → 导出。
7. 多张读图 → ImageMergeNode 拼接 → 预览 / 导出。
8. 保存工作流 → 关闭程序 → 重新加载 → 执行结果一致。

### 18.2 错误用例

1. 输入图片不存在。
2. 输入路径为空。
3. 输出目录不存在且无法创建。
4. JSON 语法错误。
5. JSON 中节点类型不存在。
6. JSON 中 edge 指向不存在节点。
7. 端口类型不匹配。
8. 工作流有环。
9. Crop 参数越界。
10. Resize 宽高为 0。
11. Blur 半径过大。
12. 中文路径图片。
13. 带空格路径图片。
14. Windows 下相对路径。
15. macOS 下相对路径。
16. 普通输入端口被连接两次。
17. 汇合节点缺少必需输入。
18. BlendNode 两张图片尺寸不匹配且未设置缩放策略。
19. JSON 参数类型错误，例如 `radius` 写成字符串。
20. 执行后再次修改参数并重复执行，旧缓存不影响新结果。

### 18.3 压力用例

1. 20 个节点的长链。
2. 50 条边的大图。
3. 10MB 图片。
4. 50MP 超大图片，要求明确拒绝或安全处理。
5. 连续执行同一工作流 10 次，不内存暴涨。

---

## 19. 需要后续开发严格遵守的规则

### 19.1 不要一次性生成整个项目

应该按阶段实现和验收：

```text
先项目骨架
再核心模型
再节点
再执行引擎
再 GUI
再命令行
再测试和部署
```

### 19.2 不要写平台专属代码

除非放在隔离模块并使用：

```cpp
#ifdef Q_OS_WIN
#endif

#ifdef Q_OS_MAC
#endif
```

基础功能不允许依赖这些宏。

### 19.3 不要写死路径

禁止：

```cpp
"/Users/xxx/Desktop"
"C:\\Users\\xxx\\Desktop"
```

必须使用：

```cpp
QFileDialog
QDir
QFileInfo
QStandardPaths
```

### 19.4 不要用裸指针管理核心对象生命周期

允许短期非拥有裸指针，但拥有关系必须明确。

推荐：

```text
std::unique_ptr
std::shared_ptr 仅在确实共享所有权时使用
QObject parent 机制
```

### 19.5 不要让 UI 直接承担业务逻辑

错误做法：

```text
MainWindow 里保存所有节点、执行所有图像处理、解析所有 JSON
```

正确做法：

```text
MainWindow 负责界面
WorkflowGraph 负责图结构
WorkflowSerializer 负责 JSON
WorkflowValidator 负责验证
ExecutionEngine 负责执行
ImageNode 子类负责具体图像操作
processing/ 中的处理器负责纯图像算法
```

GUI 层只能发起操作、显示状态和收集用户输入。节点参数校验、端口兼容性、DAG 检查、JSON 格式检查和图像执行不能散落在按钮回调里。

### 19.6 不要让节点类型依赖巨大 if-else

错误做法：

```cpp
if (type == "Crop") ...
else if (type == "Blur") ...
else if (type == "Resize") ...
```

正确做法：

```cpp
NodeFactory::registerNode("Crop", [] {
    return std::make_unique<CropNode>();
});
```

---

## 20. 最小可交付版本范围

如果时间有限，最低完成：

```text
1. Qt 主窗口
2. 节点列表
3. 节点画布
4. 能添加和移动节点
5. 能连接端口
6. 至少 8 种节点
7. JSON 保存 / 加载
8. 工作流验证
9. 一键执行
10. 错误提示不崩溃
11. Windows 重新编译通过
```

可以暂缓：

```text
复杂动画
高级节点
真正的 Git 式版本管理
外部 API
复杂多线程
OpenCV 后端
美术资源
```

---

## 21. 建议 README 中写清楚的内容

README 应该包括：

```text
项目介绍
环境要求
macOS 编译方法
Windows 编译方法
运行方法
命令行模式
示例 workflow.json
已实现节点列表
已知限制
跨平台注意事项
```

Windows 编译说明示例：

```bash
mkdir build
cd build
cmake .. -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvcxxxx_64"
cmake --build . --config Release
```

部署说明示例：

```bash
windeployqt Release/ImageNodeEditor.exe
```

---

## 22. 最终验收标准

验收前检查：

```text
[ ] macOS 能编译
[ ] Windows 能编译
[ ] 源码仓库中没有 build 目录
[ ] 没有写死 macOS / Windows 绝对路径
[ ] 所有资源能加载
[ ] 至少 8 种节点
[ ] 端口类型检查有效
[ ] 环检测有效
[ ] JSON 保存 / 加载有效
[ ] workflow.json 能回放
[ ] 输出端口可以分支连接到多个下游节点
[ ] 至少一个汇合类节点能接收多个输入并生成结果
[ ] 输入文件不存在时不崩溃
[ ] 参数错误时不崩溃
[ ] 端口类型不匹配时不崩溃
[ ] JSON 参数类型错误时不崩溃
[ ] 工作流存在环时不崩溃并给出明确提示
[ ] 输出目录不可写时不崩溃
[ ] 中文路径至少测试一次
[ ] Windows 使用 windeployqt 后能在普通目录运行
[ ] 每个核心模块能解释设计意图
```

---

## 23. 一句话架构总结

本项目应实现为：

```text
Qt GUI 节点编辑器
+ 独立 WorkflowGraph 数据模型
+ JSON 序列化 / 反序列化
+ DAG 验证和拓扑执行引擎
+ 多态 ImageNode 节点体系
+ Qt 原生图像处理
+ 跨平台 CMake 构建
+ 完整错误提示和防崩溃保护
```

不要把它做成一个所有逻辑都堆在 `MainWindow.cpp` 里的临时代码项目。
