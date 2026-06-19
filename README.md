<div align="center">

# 🎨 ImageNodeEditor

**基于 Qt 6 + C++17 的节点式图像处理工作流工具**

把读图、裁切、调色、滤波、文字叠加、预览、导出做成可视化「节点」，<br/>在画布上连线、改参数即可拼出一条图像处理流程，一键执行并保存为 JSON。

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![Qt](https://img.shields.io/badge/Qt-6.8-41CD52?logo=qt&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.16%2B-064F8C?logo=cmake&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows%20·%20macOS%20·%20Linux-lightgrey)
![Nodes](https://img.shields.io/badge/nodes-27%2B1-ff7a45)

**简体中文** ｜ [English](README.en.md)

<img src="docs/images/01-workbench.png" width="820" alt="主界面" />

</div>

---

## 目录

- [✨ 功能特性](#-功能特性)
- [🖼️ 界面预览](#️-界面预览)
- [🧰 环境要求](#-环境要求)
- [🔨 构建](#-构建)
- [▶️ 运行](#️-运行)
- [🚀 快速上手（GUI）](#-快速上手gui)
- [⌨️ 命令行模式 picdeal](#️-命令行模式-picdeal)
- [📄 工作流 JSON 格式](#-工作流-json-格式)
- [🗂️ 项目结构](#️-项目结构)
- [⚠️ 已知限制](#️-已知限制)
- [📦 第三方与许可](#-第三方与许可)

---

## ✨ 功能特性

- **可视化节点画布**：添加 / 拖动 / 删除节点，拖拽端口连线，删除连线，撤销 / 重做。
- **27 种内置节点 + 1 个宏节点**，六大类：

  | 类别 | 节点 |
  | --- | --- |
  | 输入输出 | 读入图片、导出图片、预览图片 |
  | 几何变换 | 裁切、缩放、旋转/翻转、九宫格切分 |
  | 色彩处理 | 灰度、亮度/对比度、反色、阈值、色相饱和度、通道拆分/合并 |
  | 滤波处理 | 模糊、锐化、边缘检测、像素化 |
  | 合成处理 | 文字叠加、双图混合、图片拼接、蒙版混合、图片水印 |
  | 批量处理 | 批量读入、列表取图、列表拼接、批量导出 |

- **执行引擎**：连线时检查端口方向与类型，执行前做 DAG（有向无环图）校验与拓扑排序，出错返回明确中文提示，绝不崩溃。
- **节点输出缩略图**：执行后每个节点卡片直接显示该步输出图，参数一改实时刷新。
- **工作流 JSON**：保存 / 加载流程配方（节点、位置、参数、连线），支持相对路径与中文路径。
- **进度记录与对比**：给工作流存「保存点」，并可把两个保存点（或当前画布）并排对比（缩略图 + 结构差异）。
- **拖线智能推荐**：从端口拖线到空白处，按类型筛选并推荐可连接的节点，选中即自动建节点并连线。
- **命令行 `picdeal`**：无界面执行流程，支持 ffmpeg 式线性流水线、目录批处理与类 git 的保存历史。

## 🖼️ 界面预览

| 搭建工作流（连线 + 参数） | 执行后显示输出缩略图 |
| :---: | :---: |
| <img src="docs/images/02-workflow.png" width="400" alt="连线与参数" /> | <img src="docs/images/03-executed.png" width="400" alt="执行结果" /> |

---

## 🧰 环境要求

| 依赖 | 版本 / 说明 |
| --- | --- |
| C++ 编译器 | 支持 C++17（MSVC 2022/2026、Clang、GCC 均可） |
| Qt | **6.5+**（本项目在 **6.8.3** 上开发验证）；需组件 `Core Gui Widgets Qml Quick QuickControls2 QuickWidgets`，Windows 还需 `qtshadertools` |
| CMake | 3.16+（跨平台构建用） |
| Visual Studio | 2022 / 2026（仅 Windows，走 VS 工程时需要） |

> Windows 上 Qt 推荐用 `aqtinstall` 或官方安装器装 `msvc2022_64` 套件，默认路径示例为 `C:\Qt\6.8.3\msvc2022_64`。

---

## 🔨 构建

项目提供两套等价的构建方式，**任选其一**；两者编译的是同一份源码。

<details open>
<summary><b>方式一：Visual Studio（Windows，最简单）</b></summary>

仓库自带原生 VS 解决方案，由 MSVC 直接编译，无需任何命令行步骤。

1. 安装 Qt 6.8.3 `msvc2022_64`（默认 `C:\Qt\6.8.3\msvc2022_64`）与 Visual Studio（含「使用 C++ 的桌面开发」负载）。
2. 双击根目录 **`ImageNodeEditor.slnx`**。
3. 按 **F5**（Release | x64）即可编译并运行。

构建产物在 `out/Release/`，已由 `windeployqt` 自动部署好 Qt 运行库与 QML 模块，可直接运行或拷走。

> Qt 装在别处时：设环境变量 `QTDIR`，或改 `.vcxproj` 里的 `QtDir` 默认值。
> 改动了含 `Q_OBJECT` 的头或 `qml/` 资源后，需运行 `ImageNodeEditor/GeneratedFiles/regen.bat` 刷新预生成的 moc/rcc 再编译。

</details>

<details>
<summary><b>方式二：CMake（跨平台）</b></summary>

**Windows**（生成 VS 解决方案）：

```bat
cmake -S . -B build-vs -G "Visual Studio 18 2026" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build build-vs --config Release
```

产物在 `build-vs\Release\`，也可打开 `build-vs\ImageNodeEditor.sln` 按 F5。

**macOS**：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt)
cmake --build build -j
```

产物为 `build/ImageNodeEditor.app`。

**Linux**：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.8.3/gcc_64
cmake --build build -j
```

> CMake 路径自动处理 moc/rcc（AUTOMOC/AUTORCC）并从源码编译 QtNodes，无需手动 regen。

</details>

---

## ▶️ 运行

**图形界面（GUI）**：

```bash
# Windows（VS 构建）
out\Release\ImageNodeEditor.exe
# Windows（CMake 构建）
build-vs\Release\ImageNodeEditor.exe
# macOS
open build/ImageNodeEditor.app
```

**用示例工作流快速验证**（命令行，无需界面）：

```bash
# Windows
out\Release\ImageNodeEditor.exe run ImageNodeEditor\resources\workflows\sample.json
# macOS / Linux
./build/ImageNodeEditor.app/Contents/MacOS/ImageNodeEditor run ImageNodeEditor/resources/workflows/sample.json
```

---

## 🚀 快速上手（GUI）

1. 启动程序，左侧 **节点库** 里找到「读入图片」，拖到画布。
2. 双击该节点的「图片路径」参数选一张图片。
3. 再拖入「灰度化」「模糊」「预览图片」等节点。
4. 从上一个节点的 **输出端口** 拖线到下一个节点的 **输入端口** 完成连接（拖到空白处会弹出可连接节点的推荐）。
5. 节点卡片上直接改参数。
6. 点工具栏 **执行**（或命令面板 `Ctrl+Shift+P` 搜「执行」）。每个节点会显示输出缩略图，预览节点把结果送到右侧预览区。
7. **文件 → 保存** 存成 `workflow.json`，下次 **打开** 可继续。

---

## ⌨️ 命令行模式 picdeal

构建产物旁会生成便捷的 `picdeal`（Windows 为 `picdeal.exe`，与主程序同一二进制）。也可直接用主程序 exe 加子命令。

```text
picdeal pipe   -i 输入 [--操作 [键=值|值]...] -o 输出     # ffmpeg 式线性流水线并执行
picdeal batch  -d 输入目录 -o 输出目录 [--操作 ...]        # 对目录内每张图套用同一流水线
picdeal build  -i ... [--操作 ...] --save 工作流.json      # 构建并保存为工作流（可在 GUI 打开）
picdeal run    工作流.json                                # 执行已保存的工作流
picdeal validate 工作流.json                              # 仅校验
picdeal nodes                                             # 列出所有节点与参数
picdeal save   工作流.json [-m 说明]                       # 存入保存历史（GUI「进度记录」可见）
picdeal log    [--timeline]                               # 类 git log 列出保存历史
picdeal restore <id> [-o 输出.json]                       # 回溯到某次保存
picdeal help | version
```

常用 `--操作`：`grayscale`、`blur [半径]`、`resize WxH`、`crop x= y= width= height=`、`brightness 值`、`rotate 角度`、`text 文字`、`sharpen`、`edge`、`invert`、`threshold`、`hsl`、`pixelate`、`blend 第二图`、`merge ...`、`watermark 水印图`、`preview`（完整见 `picdeal help`）。

```bash
picdeal pipe -i in.png --grayscale --blur 3 --resize 800x600 -o out.png
picdeal pipe -i a.png --blend b.png opacity=0.4 -o merged.png
picdeal batch -d photos -o out --grayscale --resize 1024x768 --format png
picdeal nodes
```

> 兼容旧入口：`ImageNodeEditor.exe --no-gui --workflow path/to/workflow.json`。

---

## 📄 工作流 JSON 格式

保存的是「流程配方」，不含图片二进制：

```json
{
  "formatVersion": 1,
  "nodes": [
    {"id": "ImageInput_1", "type": "ImageInput", "x": -240, "y": -60,
     "params": {"filePath": "../samples/input.ppm"}},
    {"id": "Grayscale_2", "type": "Grayscale", "x": 10, "y": -60, "params": {}},
    {"id": "ImageOutput_3", "type": "ImageOutput", "x": 260, "y": -60,
     "params": {"outputPath": "../samples/output.png"}}
  ],
  "edges": [
    {"fromNode": "ImageInput_1", "fromPort": "output", "toNode": "Grayscale_2", "toPort": "image"},
    {"fromNode": "Grayscale_2", "fromPort": "output", "toNode": "ImageOutput_3", "toPort": "image"}
  ]
}
```

- 图片路径优先存相对路径，加载时以 workflow 文件所在目录为基准。
- 节点类型名（`type`）见 `picdeal nodes`，端口名稳定（如 `image` / `output`）。

---

## 🗂️ 项目结构

```text
ImageNodeEditor.slnx              VS 解决方案（提交用，Windows）
CMakeLists.txt                    跨平台构建入口
ImageNodeEditor/
  main.cpp                        程序入口：判断进 GUI 还是命令行
  app/                            命令行 picdeal 的子命令调度
  core/                           基础类型（端口类型、连线、参数、结果）
  nodes/                          ImageNode 多态基类 + NodeFactory + 全部节点
  processing/                     纯图像算法（裁切/缩放/调色/滤波/合成）
  workflow/                       图结构、JSON 序列化、校验、拓扑执行、保存历史
  gui/                            主窗口、画布、参数面板、预览、对比对话框
  qml/                            QML 工作台外壳（标题栏/活动栏/侧栏）
  util/                           路径处理工具
  resources/                      示例图片与示例工作流
third_party/                      QtNodes 画布库、Codicons 图标字体
docs/images/                      README / 报告用截图
```

设计上界面、数据模型、图像算法、执行引擎分离；GUI 与命令行最终都汇到同一套 `workflow/` + `nodes/`。更详细的目录与类职责见 [`struct.md`](struct.md)，总体方案见 [`solution.md`](solution.md)。

---

## ⚠️ 已知限制

- 原生 VS 工程当前只配了 `Release | x64`（Debug 需另装 debug 版 Qt 并补一个配置）。
- 为控制内存与耗时，对超大图片、过大模糊半径等做了上限限制。
- 命令行模式用离屏渲染，依赖 `qoffscreen` 平台插件（构建已自动部署；手动拷贝运行目录时需一并带上）。
- 跨平台代码统一走 Qt API，不含平台专属系统调用；macOS 原生窗口栏在 `gui/NativeWindowChrome_mac.mm` 中单独处理。

## 📦 第三方与许可

- [QtNodes](https://github.com/paceholder/nodeeditor)：节点画布库，源码 vendored 在 `third_party/QtNodes/`，许可证见该目录。
- [Codicons](https://github.com/microsoft/vscode-codicons)：图标字体（CC-BY-4.0），见 `third_party/codicons/`。
- 本项目为课程作业，基础功能仅依赖 Qt 自带能力（`QImage` / `QPainter` 等），未引入 OpenCV 等重型依赖。
