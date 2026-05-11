# struct.md：项目结构设想与文件职责说明

本文档用于说明节点式图像处理工作流工具的推荐项目结构。后续开发者只看本文档，也应该能判断：某个功能应该放在哪个目录、哪个类负责什么、查问题时应该先看哪些文件。

---

## 1. 顶层结构

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
    core/
    nodes/
    processing/
    workflow/
    gui/
    util/

    resources/
      icons/
      samples/
      workflows/

    resources.qrc  # 可选，需要内置资源时再加入

  tests/
```

顶层文件职责：

- `CMakeLists.txt`：项目构建入口，声明 C++17、Qt 模块、源文件、资源文件和可选依赖。
- `README.md`：面向使用者，说明环境、编译、运行、示例和已知限制。
- `solution.md`：总体设计方案，说明目标、技术路线、模块划分、节点规则、执行流程和验收标准。
- `struct.md`：项目结构索引，说明目录、文件和类职责。
- `log.md`：开发和运行问题记录，记录重要修改、遇到的问题、解决方式和遗留事项。
- `agent.md`：给辅助编码工具的最终指导文件，规定编码风格、排错方式和修改边界。
- `ImageNodeEditor/`：与项目同名的源码目录，`main.cpp` 放在这里，其他模块按职责分子目录放置。
- `resources.qrc`：可选 Qt 资源入口；只有需要把图标、示例图片、示例 workflow 打包进程序时才加入。

---

## 2. `ImageNodeEditor/main.cpp`

职责：

- 创建 `QApplication`。
- 解析命令行参数。
- 判断是否进入 GUI 模式或命令行模式。
- GUI 模式创建并显示 `MainWindow`。
- 命令行模式调用 `CommandLineRunner`。

不应该做：

- 不直接解析 workflow JSON。
- 不直接执行节点。
- 不直接写图像处理逻辑。

---

## 3. `ImageNodeEditor/app/`

`app/` 放应用级协调逻辑，连接 UI、命令行入口和核心工作流系统。

```text
ImageNodeEditor/app/
  AppController.h
  AppController.cpp
  CommandLineRunner.h
  CommandLineRunner.cpp
```

### `AppController`

职责：

- 作为 GUI 层和核心 workflow 层之间的协调者。
- 维护当前打开的 workflow 文件路径。
- 处理新建、打开、保存、另存为、执行等高层操作。
- 把执行结果、错误信息、日志事件传给 `MainWindow`。

不应该做：

- 不实现具体节点算法。
- 不直接操作 `QGraphicsItem` 的绘制细节。

### `CommandLineRunner`

职责：

- 实现 `ImageNodeEditor --workflow workflow.json --no-gui`。
- 读取 workflow JSON。
- 调用 `WorkflowValidator` 和 `ExecutionEngine`。
- 把错误输出到 stderr。
- 根据结果返回退出码。

重点：

- 命令行模式和 GUI 模式必须共用 `workflow/`、`nodes/`、`processing/`。
- 命令行模式不能弹出 `QMessageBox`。

---

## 4. `ImageNodeEditor/core/`

`core/` 放最基础的数据类型，不依赖 GUI，也不依赖具体节点。

`core/` 中很多文件可以只有 `.h`，不一定有 `.cpp`。枚举、简单结构体、模板类、很短的内联函数适合 header-only；只有当某个类型有复杂非模板实现、需要隐藏实现细节或减少编译依赖时，才增加对应 `.cpp`，例如 `NodeId.cpp`。

```text
ImageNodeEditor/core/
  PortType.h
  NodeId.h
  Edge.h
  NodeParameter.h
  NodeData.h
  Result.h
```

### `PortType.h`

定义端口数据类型：

```text
ImageRGBA
ImageGray
Number
Text
Mask
ImageList
```

用于连接检查、节点输入输出声明和 JSON 校验。

### `NodeId.h`

定义节点 id 类型。可以先用 `QString` 包装，也可以用轻量结构体，目标是避免到处传无意义字符串。

### `Edge.h`

描述一条连线：

```text
fromNode
fromPort
toNode
toPort
```

只保存模型信息，不保存 UI 线条对象。

用户在画布中看到的是 `EdgeItem`，程序保存和执行时用的是这里的 `Edge`。一次端口拖拽连线成功后，`NodeScene` 应创建一条模型层 `Edge`，再创建对应的 UI 层 `EdgeItem`。

### `NodeParameter.h`

描述节点参数元数据：

```text
name
displayName
type
defaultValue
min / max
options
required
```

属性面板可以根据它自动生成控件。

### `NodeData.h`

节点执行时传递的数据容器。基础可用：

```cpp
struct NodeData {
    PortType type;
    QVariant value;
};
```

图片使用 `QImage`，数值使用 `double` 或 `int`，文本使用 `QString`。

### `Result.h`

统一返回成功或失败：

```text
ok(value)
fail(message)
```

核心层遇到错误应返回 `Result`，不要直接让程序崩溃。

---

## 5. `ImageNodeEditor/nodes/`

`nodes/` 放节点类。每个节点负责声明自己的端口、参数、保存加载参数，并调用 `processing/` 中的图像算法。

```text
ImageNodeEditor/nodes/
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
```

### `ImageNode`

所有节点的抽象基类。

必须提供：

- `typeName()`：稳定类型名，用于 JSON。
- `displayName()`：界面显示名称。
- `inputPorts()`：输入端口列表。
- `outputPorts()`：输出端口列表。
- `parameterDefinitions()`：参数元数据。
- `saveParams()`：保存参数到 JSON。
- `loadParams()`：从 JSON 加载参数并校验类型。
- `execute()`：执行节点，返回输出数据或错误。

### `NodeFactory`

职责：

- 注册节点类型。
- 根据 JSON 中的 `type` 创建节点实例。
- 查询所有可添加节点，供 `NodePalette` 使用。

规则：

- 不允许用超长 `if-else` 创建节点。
- 未知节点类型必须返回错误。
- 新增节点时必须注册到 `NodeFactory`。

### 基础节点职责

- `ImageInputNode`：读取图片，输出 `ImageRGBA`。UI 中必须提供 `QFileDialog` 选择外部图片；节点参数保存图片路径。可选把外部图片复制到 workflow 同级 `assets/` 目录，并在 JSON 中保存相对路径，便于复用和迁移。
- `ImageOutputNode`：接收图片并保存到文件。
- `PreviewNode`：透传图片，并把结果交给 UI 预览。
- `CropNode`：按矩形裁切图片。
- `ResizeNode`：缩放图片。
- `GrayscaleNode`：转换为灰度图。
- `BrightnessContrastNode`：调整亮度和对比度。
- `GaussianBlurNode`：模糊图片；如果实现的是 box blur，节点名称可用 `BlurNode`。
- `RotateFlipNode`：旋转或翻转图片。
- `TextOverlayNode`：在图片上绘制文字。
- `BlendNode`：接收两张图片并按透明度混合，用于分支汇合。
- `ImageMergeNode`：接收多张图片并横向、纵向或网格拼接。

节点类不应该做：

- 不直接操作 `QGraphicsScene`。
- 不弹出 UI 对话框。
- 不写平台专属路径。
- 不把复杂像素算法堆在 `execute()` 中；复杂算法应放到 `processing/`。

---

## 6. `ImageNodeEditor/processing/`

`processing/` 放纯图像处理算法，尽量写成无状态函数或轻量工具类。

```text
ImageNodeEditor/processing/
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
```

### `CropProcessor`

负责裁切逻辑：

- 检查裁切矩形。
- 调用 `QImage::copy()`。
- 返回裁切结果或错误。

### `ResizeProcessor`

负责缩放逻辑：

- 检查宽高。
- 处理是否保持比例。
- 调用 `QImage::scaled()`。

### `ColorProcessor`

负责色彩处理：

- 灰度化。
- 亮度调整。
- 对比度调整。
- 后续可扩展通道拆分、通道合并、色彩统计。

### `BlurProcessor`

负责滤波：

- 半径为 0 时直接返回原图。
- 基础版可以实现简单 box blur。
- 必须限制半径和图片尺寸，避免长时间卡死。

### `ComposeProcessor`

负责图像合成：

- 文字叠加。
- 双图混合。
- 多图拼接。
- 后续可扩展蒙版合成。

processing 层不应该依赖：

- `MainWindow`
- `NodeScene`
- `QGraphicsItem`
- `QMessageBox`

---

## 7. `ImageNodeEditor/workflow/`

`workflow/` 是项目核心，负责工作流结构、验证、序列化和执行。

```text
ImageNodeEditor/workflow/
  WorkflowGraph.h
  WorkflowGraph.cpp
  WorkflowSerializer.h
  WorkflowSerializer.cpp
  WorkflowValidator.h
  WorkflowValidator.cpp
  ExecutionEngine.h
  ExecutionEngine.cpp
```

### `WorkflowGraph`

职责：

- 保存节点集合。
- 保存边集合。
- 添加、删除、查找节点。
- 添加、删除、查找边。
- 查询某节点的上游和下游。
- 保证模型层数据和 UI 层显示可以互相同步。

注意：

- 保存的是业务图，不是 `QGraphicsItem`。
- UI 层删除节点时，应先通过 `WorkflowGraph` 删除相关边。

### `WorkflowSerializer`

职责：

- 把 `WorkflowGraph` 保存为 JSON。
- 从 JSON 重建 `WorkflowGraph`。
- 处理 `formatVersion`。
- 处理节点位置、类型、参数、连线。
- 处理相对路径和 workflow 文件所在目录。

错误处理：

- JSON 语法错误要包含解析位置。
- 未知节点类型要明确提示。
- 缺字段、字段类型错误、端口不存在都不能崩溃。

### `WorkflowValidator`

职责：

- 检查节点 id 唯一。
- 检查边指向的节点和端口存在。
- 检查端口方向。
- 检查端口类型兼容。
- 检查普通输入端口不能被多条边连接。
- 检查必需输入是否存在。
- 检查参数是否合法。
- 检查文件路径和输出目录。
- 检查图是否为 DAG。

### `ExecutionEngine`

职责：

- 调用 `WorkflowValidator`。
- 对图做拓扑排序。
- 按依赖顺序执行节点。
- 为每个节点收集输入。
- 缓存每个节点每个输出端口的数据。
- 遇到错误立即停止并返回错误。
- 记录执行日志和节点状态。

缓存建议：

```cpp
QMap<NodeId, QMap<QString, NodeData>> nodeOutputs;
```

---

## 8. `ImageNodeEditor/gui/`

`gui/` 放 Qt Widgets 图形界面。

```text
ImageNodeEditor/gui/
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
```

### `MainWindow`

职责：

- 搭建主界面布局。
- 创建菜单栏和工具栏。
- 管理 `NodePalette`、`NodeScene`、`PropertyPanel`、`PreviewPanel`、`LogPanel`。
- 把用户操作转发给 `AppController` 或 `WorkflowGraph`。

不应该做：

- 不实现图像处理算法。
- 不直接解析 JSON。
- 不直接做拓扑排序。

### `NodeScene`

职责：

- 管理画布交互。
- 添加、移动、删除 `NodeItem`。
- 创建和删除 `EdgeItem`。
- 处理端口拖拽连线。
- 把连线请求交给 `WorkflowValidator` 或 `WorkflowGraph` 检查。

### `NodeItem`

职责：

- 显示一个节点矩形。
- 显示节点标题、端口、执行状态。
- 保存对应的 `nodeId`。
- 节点位置变化时同步到 `WorkflowGraph`。

### `PortItem`

职责：

- 显示输入 / 输出端口。
- 保存端口名、方向、类型。
- 处理鼠标点击或拖拽开始连线。

### `EdgeItem`

职责：

- 显示端口之间的连线。
- 根据两端端口位置更新路径。
- 只保存 UI 线条信息和对应 edge id，不保存业务数据。

### `PropertyPanel`

职责：

- 选中节点后显示参数控件。
- 根据节点参数元数据生成输入框、下拉框、颜色选择、文件选择等控件。
- 修改参数后更新 `WorkflowGraph` 并标记 dirty。

### `PreviewPanel`

职责：

- 显示选中节点或最终输出图片。
- 支持适应窗口缩放。
- 空结果时显示明确提示。

### `NodePalette`

职责：

- 显示可添加节点列表。
- 从 `NodeFactory` 获取节点类型和分类。
- 双击或拖拽添加节点到画布。

### `LogPanel`

职责：

- 显示运行日志。
- 显示验证错误、执行错误、保存加载结果。
- 便于用户定位当前流程失败原因。

---

## 9. `ImageNodeEditor/util/`

`util/` 放跨模块通用工具。

```text
ImageNodeEditor/util/
  PathUtils.h
  PathUtils.cpp
  ImageUtils.h
  ImageUtils.cpp
  JsonUtils.h
  JsonUtils.cpp
```

### `PathUtils`

职责：

- 相对路径和绝对路径转换。
- 以 workflow 文件所在目录解析资源路径。
- 检查文件存在、目录可写。
- 使用 `QDir`、`QFileInfo`、`QStandardPaths`，避免手工拼接路径。

### `ImageUtils`

职责：

- 图片格式转换。
- 空图检查。
- 图片尺寸限制。
- `QImage` 安全复制。
- 常用颜色和像素辅助函数。

### `JsonUtils`

职责：

- 安全读取 JSON 字段。
- 检查字段类型。
- 生成统一错误信息。
- 减少 `WorkflowSerializer` 中重复的 contains/type 判断。

---

## 10. `ImageNodeEditor/resources/`

```text
ImageNodeEditor/resources/
  icons/
  samples/
  workflows/
```

- `icons/`：菜单、工具栏、节点图标。
- `samples/`：示例图片。
- `workflows/`：示例 workflow JSON。

规则：

- `resources.qrc` 是可选的；基础版可以只用文字按钮或 Qt 标准图标。
- 如果使用自定义图标和示例资源，再把它们加入 `resources.qrc`。
- 使用 Qt Resource System 后，代码中用 `:/icons/...`、`:/samples/...` 访问资源。
- Qt 自带标准图标可通过 `QStyle::standardIcon()` 获取，适合基础版本。
- 自定义图标可使用自己绘制的 SVG / PNG 或授权清楚的开源图标。

---

## 10.1 用户 workflow 项目目录

用户从外部导入图片后，可以选择复制到 workflow 同级的 `assets/` 目录。这个目录是用户项目数据，不是源码目录的一部分。

```text
MyWorkflowProject/
  workflow.json
  assets/
    input_orange.png
    logo.png
  output/
    result.png
```

规则：

- `ImageInputNode` 保存 `assets/input_orange.png` 这样的相对路径。
- `WorkflowSerializer` 加载时以 `workflow.json` 所在目录为基准解析相对路径。
- 如果 assets 中的图片缺失，验证阶段提示用户重新选择图片。

---

## 11. `tests/`

```text
tests/
  test_workflow_json.cpp
  test_graph_validation.cpp
  test_nodes.cpp
```

### `test_workflow_json.cpp`

测试：

- 保存 JSON。
- 加载 JSON。
- 缺字段。
- 未知节点类型。
- 参数类型错误。
- 相对路径解析。

### `test_graph_validation.cpp`

测试：

- 合法线性图。
- 合法分支图。
- 合法汇合图。
- 环检测。
- 端口方向错误。
- 端口类型不匹配。
- 普通输入端口多连线。

### `test_nodes.cpp`

测试：

- 每个基础节点正常执行。
- 空输入。
- 参数越界。
- 图片尺寸过大。
- 保存失败。

---

## 12. 查找问题时的入口

```text
程序打不开
→ main.cpp / AppController / CMakeLists.txt

GUI 按钮或菜单没反应
→ MainWindow / AppController

节点显示、拖拽、连线问题
→ NodeScene / NodeItem / PortItem / EdgeItem

参数面板不对
→ PropertyPanel / NodeParameter / ImageNode parameterDefinitions

JSON 保存加载错误
→ WorkflowSerializer / JsonUtils / NodeFactory

端口不能连接或错误连接
→ WorkflowValidator / PortInfo / PortType

执行顺序不对或环检测失败
→ WorkflowValidator / ExecutionEngine

某个图像处理结果错误
→ 对应 Node 类 / processing 中对应 Processor

路径、中文文件名、相对路径问题
→ PathUtils / WorkflowSerializer / ImageInputNode / ImageOutputNode

程序崩溃
→ 先看 log.md 记录，再检查 Result 返回链路、空指针、QGraphicsItem 生命周期
```
