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

- [简介](#简介)
- [功能特性](#功能特性)
- [界面预览](#界面预览)
- [环境要求](#环境要求)
- [安装与构建](#安装与构建)
- [运行](#运行)
- [界面总览](#界面总览)
- [使用指南](#使用指南)
  - [1. 添加与管理节点](#1-添加与管理节点)
  - [2. 连接节点](#2-连接节点)
  - [3. 编辑参数](#3-编辑参数)
  - [4. 执行工作流](#4-执行工作流)
  - [5. 画布导航](#5-画布导航)
  - [6. 撤销与重做](#6-撤销与重做)
  - [7. 多画布标签](#7-多画布标签)
  - [8. 宏节点](#8-宏节点)
  - [9. 方案库（模板）](#9-方案库模板)
  - [10. 进度记录与版本对比](#10-进度记录与版本对比)
  - [11. 命令面板与快速访问](#11-命令面板与快速访问)
  - [12. 保存、打开与导出](#12-保存打开与导出)
  - [13. 拖放](#13-拖放)
  - [14. 主题与外观](#14-主题与外观)
- [设置](#设置)
- [快捷键](#快捷键)
- [节点参考](#节点参考)
- [命令行模式 picdeal](#命令行模式-picdeal)
- [工作流 JSON 格式](#工作流-json-格式)
- [项目结构](#项目结构)
- [常见问题与已知限制](#常见问题与已知限制)
- [第三方与许可](#第三方与许可)

---

## 简介

ImageNodeEditor 是一个桌面端的**节点式图像处理工作流工具**。每个图像操作是一个「节点」，节点有输入、输出端口和可调参数；用连线把节点接起来就构成一条处理流程（工作流）。流程可以一键执行、实时预览、保存成 JSON，也可以用命令行 `picdeal` 在无界面环境下批量执行。

界面（GUI）与命令行共用同一套核心（图结构、校验、执行引擎、节点工厂），行为一致。

## 功能特性

- **可视化节点画布**：添加 / 拖动 / 复制 / 删除节点，拖拽端口连线，删除连线，撤销 / 重做，多画布标签。
- **27 种内置节点 + 1 个宏节点**，覆盖输入输出、几何变换、色彩处理、滤波处理、合成处理、批量处理六大类（完整清单见[节点参考](#节点参考)）。
- **可靠的执行引擎**：连线时校验端口方向与类型，执行前做 DAG 校验与拓扑排序，错误返回明确中文提示而非崩溃；节点级缓存避免重复计算。
- **所见即所得**：执行后每个节点直接显示输出缩略图；预览面板支持适应窗口、点击放大、像素取色。
- **工作流持久化**：保存 / 加载 JSON 配方，支持相对路径与中文路径。
- **进度记录与对比**：保存点、时间线、恢复、分支，以及两个版本的并排可视化对比。
- **效率工具**：命令面板、快捷添加节点、拖线智能推荐、方案库模板、整理画布、宏节点封装。
- **命令行 `picdeal`**：ffmpeg 式线性流水线、目录批处理、类 git 的保存历史。
- **深色 / 浅色主题**、界面缩放、跨平台（Windows / macOS / Linux）。

## 界面预览

| 搭建工作流（连线 + 参数） | 执行后显示输出缩略图 |
| :---: | :---: |
| <img src="docs/images/02-workflow.png" width="400" alt="连线与参数" /> | <img src="docs/images/03-executed.png" width="400" alt="执行结果" /> |

---

## 环境要求

| 依赖 | 版本 / 说明 |
| --- | --- |
| C++ 编译器 | 支持 C++17（MSVC 2022/2026、Clang、GCC 均可） |
| Qt | **6.5+**（本项目在 **6.8.3** 上开发验证）；组件 `Core Gui Widgets Qml Quick QuickControls2 QuickWidgets`，Windows 还需 `qtshadertools` |
| CMake | 3.16+（跨平台构建用） |
| Visual Studio | 2022 / 2026（仅 Windows，走 VS 工程时需要） |

> Windows 上推荐用 `aqtinstall` 或官方安装器装 Qt `msvc2022_64` 套件，默认路径示例 `C:\Qt\6.8.3\msvc2022_64`。

---

## 安装与构建

两套等价构建方式，**任选其一**；编译的是同一份源码。

<details open>
<summary><b>方式一：Visual Studio（Windows，最简单）</b></summary>

1. 安装 Qt 6.8.3 `msvc2022_64`（默认 `C:\Qt\6.8.3\msvc2022_64`）与 Visual Studio（含「使用 C++ 的桌面开发」负载）。
2. 双击根目录 **`ImageNodeEditor.slnx`**。
3. 按 **F5**（Release | x64）即可编译并运行。

产物在 `out/Release/`，已由 `windeployqt` 自动部署好 Qt 运行库与 QML 模块。

> Qt 装在别处时：设环境变量 `QTDIR`，或改 `.vcxproj` 里的 `QtDir` 默认值。
> 改动了含 `Q_OBJECT` 的头或 `qml/` 资源后，运行 `ImageNodeEditor/GeneratedFiles/regen.bat` 刷新预生成的 moc/rcc 再编译。

</details>

<details>
<summary><b>方式二：CMake（跨平台）</b></summary>

**Windows**：

```bat
cmake -S . -B build-vs -G "Visual Studio 18 2026" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build build-vs --config Release
```

**macOS**：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt)
cmake --build build -j
```

**Linux**：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.8.3/gcc_64
cmake --build build -j
```

> CMake 路径自动处理 moc/rcc（AUTOMOC/AUTORCC）并从源码编译 QtNodes，无需手动 regen。

</details>

---

## 运行

```bash
# 图形界面
out\Release\ImageNodeEditor.exe            # Windows（VS 构建）
build-vs\Release\ImageNodeEditor.exe       # Windows（CMake 构建）
open build/ImageNodeEditor.app             # macOS

# 用示例工作流快速验证（无界面）
out\Release\ImageNodeEditor.exe run ImageNodeEditor\resources\workflows\sample.json
```

---

## 界面总览

启动后界面分为几个区域：

| 区域 | 说明 |
| --- | --- |
| **顶部标题栏 / 菜单** | 左侧六个菜单（文件 / 编辑 / 节点操作 / 视图 / 设置 / 布局）、前进后退、文档标题、右侧执行 / 预览 / 面板 / 窗口按钮。 |
| **活动栏**（最左竖排图标） | 切换左侧栏内容：节点库、工作流大纲、方案库、进度记录、搜索与快速访问、问题、运行诊断；底部为命令面板、预览开关、设置。 |
| **左侧栏** | 跟随活动栏切换；最常用的是「节点库」（可搜索、拖拽添加节点）。 |
| **中央画布** | 摆放节点、连线的主区域；顶部有画布标签条和宏层级面包屑。 |
| **右侧预览** | 显示选中节点或最终输出图片，可适应窗口、点击放大。 |
| **底部面板** | 终端、问题、输出/日志等标签页。 |
| **状态栏** | 显示就绪 / 执行状态、当前选中、缩放等信息。 |
| **小地图** | 画布左下角缩略导航（可在设置中开关）。 |

---

## 使用指南

### 1. 添加与管理节点

- **从节点库添加**：在左侧「节点库」搜索或浏览，把节点**拖到画布**即可创建。
- **快捷添加**：按 `Ctrl+K`，或在画布上按 `Tab`，弹出搜索框，输入节点中文名 / 英文类型名 / 分类，回车创建到鼠标位置。
- **移动**：拖动节点；松手时可吸附到对齐参考线或网格（见[设置](#设置)）。
- **复制 / 删除**：右键节点选择「复制执行节点 / 删除执行节点」，或选中后按 `Delete` / `Backspace` 删除。复制会连同参数一起复制。
- **节点卡片**：每个节点常驻显示自己的全部参数；执行后还会显示输出缩略图。

### 2. 连接节点

- 从一个节点的**输出端口**按住拖到另一个节点的**输入端口**松手即可连线。
- **端口规则**：输出只能连输入；类型不兼容不能连（端口按数据类型着色）；普通输入端口只能接一条边，输出端口可分支给多个下游；会形成环的连线会被拒绝。
- **拖线智能推荐**：从端口拖线**松开在空白处**，程序按端口类型筛出能接的节点、按「常用加工在前」排序弹出推荐，选中即自动建节点并连好线（一步可撤销）。
- **删除连线**：单击选中连线后按 `Delete`，或右键连线选择删除。

### 3. 编辑参数

- 节点卡片上直接编辑参数：整数 / 小数用数字框，布尔用复选框，选项用下拉框，文本用输入框，文件 / 目录 / 颜色用带「…」按钮的选择器。
- 改完参数后会**防抖触发实时预览**，对应节点及其上游会重新计算并刷新缩略图与预览。
- 各参数的取值范围与默认值见[节点参考](#节点参考)，或运行 `picdeal nodes` 查看。

### 4. 执行工作流

- 点顶部 **执行**（或命令面板搜「执行」、运行诊断面板里的按钮）运行整条流程。
- 执行前自动**校验**：端口、类型、必填输入、参数范围、是否为 DAG；不通过会在「问题」面板给出明确提示并定位到出错节点。
- 执行时节点显示**运行状态**（执行中 / 成功 / 失败 / 命中缓存）与耗时；失败节点会自动选中并居中。
- 执行可**取消**：执行期间「执行」按钮变为取消入口。
- 节点级**缓存**：没改动的节点复用上次结果，连续执行同一流程更快。
- 结果查看：预览节点把结果送到右侧预览；导出节点把结果写文件；每个节点卡片显示输出缩略图。

### 5. 画布导航

- **平移**：在空白处按住拖动画布。
- **缩放**：鼠标滚轮缩放（低速步进，速度可在设置调）；范围 25%–300%。
- **整理画布**：菜单 / 命令面板的「整理画布」会按依赖层级自动排布节点。
- **适应视图**：画布右键「重置视图（适应全部节点）」把所有节点收进视野。
- **吸附**：拖动节点松手时可吸附到与其他节点对齐的参考线，或对齐到网格（设置可开关）。
- **小地图**：左下角缩略图导航，可在设置开关。

### 6. 撤销与重做

添加、复制、删除节点 / 连线、移动节点、修改参数、套用模板、恢复进度等都进入撤销栈；`Ctrl+Z` 撤销、`Ctrl+Y` 重做。窗口标题会标出未保存状态。

### 7. 多画布标签

画布上方的标签条支持多个画布；点末尾的「+」新建画布，可在多条工作流之间切换，互不干扰。

### 8. 宏节点

把一组节点**封装成一个宏节点**，让复杂流程更整洁：

- 选中若干节点 → 菜单 / 命令面板「封装为宏」。宏会按内部「自由端口」自动生成对外的输入 / 输出端口，可像普通节点一样连线。
- **进入 / 返回**：双击宏节点进入其内部子图编辑；用画布上方的**面包屑**（主画布 › 宏A › …）逐级返回。
- 修改宏内部后端口会随之更新。

### 9. 方案库（模板）

活动栏「方案库」用于保存可直接套用的流程模板：

- **保存当前为模板** / **导入模板**（从外部 JSON 导入）。
- 每个模板可 **套用 / 改名 / 导出 / 删除**（预设方案只读，只能套用 / 导出）。
- 套用模板会覆盖当前画布，但可撤销。

### 10. 进度记录与版本对比

活动栏「进度记录」类似轻量版本管理：

- **时间线**：每次保存（`Ctrl+S`）自动记一条，可一键「恢复」回到那次保存。
- **保存点**：手动「保存当前进度」，每条可 **恢复 / 分支 / 改名 / 导出 / 删除**；分支用于从某个保存点拉出新流程。
- **对比**：点「对比…」打开版本对比对话框——上方并排显示两个版本（保存点或当前画布）的**输出缩略图**，下方按 新增 / 删除节点、参数从旧值改成新值、连线增删 列出**结构差异**。

GUI 的进度记录与命令行 `picdeal save/log/restore` 共用同一套存储，互相可见。

### 11. 命令面板与快速访问

按 `Ctrl+Shift+P` 打开命令面板，输入关键字可搜索并执行命令、跳转节点、定位问题、打开最近工作流。是不熟悉菜单时最快的入口。

### 12. 保存、打开与导出

- **新建 / 打开 / 保存 / 另存为**：文件菜单或命令面板。保存的是工作流 JSON（不含图片二进制）。
- **导出工作流**：导出一份工作流副本。
- **导出预览结果**：把当前预览图片存成 PNG。
- **导出画布截图**（`Ctrl+Shift+E`）：把整张画布渲染成图片。

### 13. 拖放

- 把**图片文件**拖进画布 → 自动创建「读入图片」节点（多张会级联摆放）。
- 把 **`.json` 工作流**拖进画布 → 直接打开该工作流。

### 14. 主题与外观

支持**深色 / 浅色 / 跟随系统**三种主题（默认深色），以及整体界面缩放，均在设置中切换并持久化保存。

---

## 设置

通过菜单「设置 → 打开设置」进入，分为几页：

| 页 | 项 | 说明 |
| --- | --- | --- |
| **外观** | 主题 | 深色 / 浅色 / 跟随系统 |
| | 界面大小 | 菜单、侧栏、按钮、节点参数控件、文字的整体缩放 |
| **画布** | 当前画布缩放 | 只影响中央画布 |
| | 滚轮缩放速度 | 每滚一格的缩放比例（默认 4%） |
| | 吸附到对齐参考线 | 拖动节点松手时吸附到与其他节点对齐的位置（默认开） |
| | 节点对齐到网格 | 松手时对齐到 20px 网格（默认关） |
| | 节点卡片显示输出缩略图 | 执行后在节点上显示输出图（默认开） |
| **工作台** | 显示右侧预览 / 显示底部面板 / 显示画布小地图 | 各区域显隐 |
| | 重置布局 | 把侧栏、预览、底部面板恢复默认排布 |
| **快捷键** | 快捷键查询 | 查看当前命令与快捷键 |

设置持久化保存（基于 `QSettings`），下次启动自动恢复。

---

## 快捷键

| 快捷键 | 功能 |
| --- | --- |
| `Ctrl+K` / 画布内 `Tab` | 快捷添加节点 |
| `Ctrl+Shift+P` | 命令面板 |
| `Ctrl+Z` / `Ctrl+Y` | 撤销 / 重做 |
| `Delete` / `Backspace` | 删除选中的节点或连线 |
| `Ctrl+=` / `Ctrl+-` / `Ctrl+0` | 界面放大 / 缩小 / 恢复默认 |
| `Ctrl+Shift+E` | 导出画布截图 |
| 鼠标滚轮 | 画布缩放 |
| 空白处拖动 | 平移画布 |
| `Esc` | 关闭命令面板等弹层 |

> 新建 / 打开 / 保存 / 执行 等可通过菜单或命令面板触发；保存请用文件菜单的「保存」。

---

## 节点参考

共 **27 个内置节点 + 1 个宏节点**。参数列出「名称（类型）取值」，完整定义可运行 `picdeal nodes` 查看。

### 输入输出

| 节点 | 说明 | 参数 |
| --- | --- | --- |
| 读入图片 `ImageInput` | 从文件读入图片，输出 RGBA | `filePath` 输入文件 |
| 导出图片 `ImageOutput` | 把图片保存到文件 | `outputPath` 默认 `output.png` |
| 预览图片 `Preview` | 透传图片并送到预览区 | 无 |

### 几何变换

| 节点 | 说明 | 参数 |
| --- | --- | --- |
| 裁切 `Crop` | 按矩形裁切 | `x/y` 0~100000，`width/height` 1~100000（默认 200） |
| 缩放 `Resize` | 缩放图片 | `width`(800)/`height`(600) 1~100000，`keepAspect` 布尔(true) |
| 旋转/翻转 `RotateFlip` | 旋转或翻转 | `angle` [0/90/180/270]，`flipHorizontal`/`flipVertical` 布尔 |
| 九宫格切分 `GridSplit` | 切成网格，固定 9 个输出口 | `rows`/`columns` 1~3（默认 3） |

### 色彩处理

| 节点 | 说明 | 参数 |
| --- | --- | --- |
| 灰度化 `Grayscale` | 转灰度 | 无 |
| 亮度/对比度 `BrightnessContrast` | 调亮度与对比度 | `brightness` -255~255，`contrast` 0~4(1.0) |
| 反色 `Invert` | 反相 | 无 |
| 阈值二值化 `Threshold` | 按阈值二值化 | `level` 0~255(128) |
| 色相/饱和度 `HueSaturation` | HSL 调整 | `hue` -180~180，`saturation`/`lightness` -100~100 |
| 通道拆分 `ChannelSplit` | 拆出通道 | 无 |
| 通道合并 `ChannelMerge` | 合并通道 | 无 |

### 滤波处理

| 节点 | 说明 | 参数 |
| --- | --- | --- |
| 模糊 `Blur` | 均值模糊（滑动窗口） | `radius` 0~20(3) |
| 锐化 `Sharpen` | 锐化 | `amount` 0~3(1.0)，`radius` 1~10(2) |
| 边缘检测 `EdgeDetect` | 边缘检测 | 无 |
| 像素化 `Pixelate` | 马赛克 | `blockSize` 2~100(8) |

### 合成处理

| 节点 | 说明 | 参数 |
| --- | --- | --- |
| 文字叠加 `TextOverlay` | 在图上画文字 | `text`、`anchor`、`x/y`、`size` 1~256(32)、`color`、`fontFamily`、`bold`、`opacity` 0~1、`outline`、`outlineColor` |
| 双图混合 `Blend` | 两图按模式混合 | `opacity` 0~1(0.5)、`mode`[normal/multiply/screen/overlay/darken/lighten/difference]、`sizeMode`[stretch/fit/error] |
| 图片拼接 `ImageMerge` | 多图拼接 | `mode`[horizontal/vertical/grid]、`columns` 1~9(2)、`background` 颜色 |
| 蒙版混合 `MaskBlend` | 按蒙版混合两图 | 无 |
| 图片水印 `ImageOverlay` | 叠加水印图 | `anchor`、`offsetX/offsetY`、`scale` 1~400(100)、`opacity` 0~1(0.8) |

### 批量处理（ImageList 数据类型，端口仅与同类互连）

| 节点 | 说明 | 参数 |
| --- | --- | --- |
| 批量读入 `FolderInput` | 读目录为图片列表 | `dirPath` 目录、`maxCount` 1~64(16) |
| 列表取图 `ListPick` | 从列表取第 N 张回到单图 | `index` 1~64(1) |
| 列表拼接 `ListMerge` | 把列表拼成单图 | `mode`、`columns` 1~9(3)、`background` |
| 批量导出 `ListExport` | 把列表批量存盘 | `outputDir` 目录、`baseName`(image)、`format`[png/jpg] |

### 高级功能

| 节点 | 说明 | 参数 |
| --- | --- | --- |
| 宏节点 `Macro` | 封装一组节点为子图 | `displayName` 显示名 |

---

## 命令行模式 picdeal

构建产物旁会生成便捷的 `picdeal`（Windows 为 `picdeal.exe`，与主程序同一二进制）。

```text
picdeal pipe   -i 输入 [--操作 [键=值|值]...] -o 输出     # ffmpeg 式线性流水线并执行
picdeal batch  -d 输入目录 -o 输出目录 [--操作 ...] [--format png] [--max N]
                                                          # 对目录内每张图套用同一流水线
picdeal build  -i ... [--操作 ...] --save 工作流.json      # 构建并保存为工作流（可在 GUI 打开）
picdeal run    工作流.json                                # 执行已保存的工作流
picdeal validate 工作流.json                              # 仅校验
picdeal nodes                                             # 列出所有节点与参数
picdeal save   工作流.json [-m 说明]                       # 存入保存历史（GUI「进度记录」可见）
picdeal log    [--timeline]                               # 类 git log 列出保存历史
picdeal restore <id> [-o 输出.json]                       # 回溯到某次保存
picdeal help | version
```

**`--操作`（与 GUI 节点对应）**：`grayscale`/`gray`、`blur [半径]`、`resize [WxH | width= height= keepAspect=]`、`crop [x= y= width= height=]`、`brightness`/`bc [值 | brightness= contrast=]`、`rotate [角度 | angle= flipHorizontal= flipVertical=]`、`text [文字 | text= x= y= size= color=]`、`preview`、`blend 第二图 [opacity= mode=]`、`merge 第二图 [第三图] [mode= columns=]`、`sharpen [强度 | amount= radius=]`、`edge`/`edgedetect`、`invert`、`threshold [阈值 | level=]`、`hsl [色相 | hue= saturation= lightness=]`、`pixelate [块大小 | blockSize=]`、`watermark 水印图 [anchor= offsetX= offsetY= scale= opacity=]`。

**示例**：

```bash
picdeal pipe -i in.png --grayscale --blur 3 --resize 800x600 -o out.png
picdeal pipe -i a.png --blend b.png opacity=0.4 mode=multiply -o merged.png
picdeal batch -d photos -o out --grayscale --resize 1024x768 --format png
picdeal build -i in.png --brightness 20 contrast=1.2 --save flow.json
picdeal save flow.json -m "初版" && picdeal log
```

> 兼容旧入口：`ImageNodeEditor.exe --no-gui --workflow path/to/workflow.json`。

---

## 工作流 JSON 格式

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

- `formatVersion`：格式版本。`nodes`：节点的 `id` / `type` / 坐标 / `params`。`edges`：连线的 `fromNode·fromPort → toNode·toPort`。
- 图片路径优先存相对路径，加载时以 workflow 文件所在目录为基准；支持中文与带空格路径。
- 节点类型名见 `picdeal nodes`，端口名稳定（如 `image` / `output`）。

---

## 项目结构

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

## 常见问题与已知限制

- **侧边栏黑屏**：多见于手动拷贝运行目录但缺了 QML 模块。请用 `windeployqt --qmldir` 部署，或直接用构建产物目录。
- **命令行启动崩溃 `0xC0000409`**：缺 `qoffscreen` 平台插件。构建已自动部署；手动拷贝时需带上 `platforms/qoffscreen.dll`。
- **找不到 Qt**：检查 `CMAKE_PREFIX_PATH` / `QTDIR` 是否指向正确的 Qt 套件。
- **VS 改了 `Q_OBJECT` 头或 qml 后缺符号**：运行 `ImageNodeEditor/GeneratedFiles/regen.bat` 刷新 moc/rcc（CMake 路径用 AUTOMOC，无需此步）。
- 原生 VS 工程只配了 `Release | x64`；为控制内存与耗时，对超大图片、过大模糊半径等做了上限限制。

## 第三方与许可

- [QtNodes](https://github.com/paceholder/nodeeditor)：节点画布库，源码 vendored 在 `third_party/QtNodes/`，许可证见该目录。
- [Codicons](https://github.com/microsoft/vscode-codicons)：图标字体（CC-BY-4.0），见 `third_party/codicons/`。
- 本项目为课程作业，基础功能仅依赖 Qt 自带能力（`QImage` / `QPainter` 等），未引入 OpenCV 等重型依赖。
