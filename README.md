# ImageNodeEditor

Qt 6 Widgets + C++17 节点式图像处理工作流原型。

## 构建

macOS Homebrew Qt 示例：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt
cmake --build build
```

## 运行

GUI：

```bash
# macOS 双击 build/ImageNodeEditor.app，或在 Finder 中打开该 app
# Windows 运行 build\Release\ImageNodeEditor.exe 或 build\Debug\ImageNodeEditor.exe
```

命令行执行 workflow：

```bash
# macOS:
./build/ImageNodeEditor.app/Contents/MacOS/ImageNodeEditor --no-gui --workflow path/to/workflow.json

# Windows:
build\Release\ImageNodeEditor.exe --no-gui --workflow path\to\workflow.json
```

运行示例 workflow：

```bash
./build/ImageNodeEditor.app/Contents/MacOS/ImageNodeEditor --no-gui --workflow ImageNodeEditor/resources/workflows/sample.json
```

## Windows / Visual Studio 提交结构

仓库自带原生 Visual Studio 2026 解决方案，**由 VS/MSVC 直接编译本项目源码**（不再委托 CMake）：

```text
ImageNodeEditor.slnx                            根目录解决方案（VS 2026 新格式）
ImageNodeEditor/ImageNodeEditor.vcxproj         同名项目（含 main.cpp 等全部源码）
ImageNodeEditor/ImageNodeEditor.vcxproj.filters
ImageNodeEditor/ImageNodeEditor.vcxproj.user
ImageNodeEditor/GeneratedFiles/                 Qt moc/rcc 预生成产物 + regen.bat
third_party/QtNodes/prebuilt/QtNodes.lib        第三方节点库（预编译静态库）
```

**用法：双击根目录 `ImageNodeEditor.slnx` → 在 VS 里按 F5（开始执行）即可编译并运行。**

工作机制：

- `ImageNodeEditor/` 下的全部 `.cpp` 由 MSVC 直接编译。
- Qt 的 moc/rcc 产物已**预生成**在 `ImageNodeEditor/GeneratedFiles/`，作为普通源码参与编译，因此 VS 构建时不依赖任何 Qt 命令行工具。改了含 `Q_OBJECT` 的头（`CanvasTabBar.h` / `WorkbenchHostWidget.h` / `WorkbenchModels.h`）或 `qml/` 资源后，运行 `ImageNodeEditor/GeneratedFiles/regen.bat` 刷新即可。
- 第三方 **QtNodes** 以预编译静态库 `third_party/QtNodes/prebuilt/QtNodes.lib` 链接（非本项目源码，避免把第三方整库搬进工程）。
- 链接后由 `windeployqt`（带 `--qmldir` 扫描 qml 源码）自动把 Qt6 运行库、平台插件**以及 Qt Quick/QML 模块**部署到 `out/Release/`，F5 与双击 exe 均可直接运行。（缺 QML 模块会导致工作台侧边栏黑屏。）
- 仅 `Release|x64` 配置（本机只装了 Release 版 Qt）。

前置条件：装好 Qt 6.8.3 msvc2022_64（默认 `C:\Qt\6.8.3\msvc2022_64`，可用环境变量 `QTDIR` 覆盖）与 VS 2026（含 C++ 桌面开发负载）。换机器若 Qt 路径不同，设 `QTDIR` 或改 `.vcxproj` / `.vcxproj.user` 中的 `QtDir` 默认值即可。

> 跨平台（macOS / Linux）以及重新编译 QtNodes 仍以根目录 `CMakeLists.txt` 为准。纯 CMake 构建：

```bat
cmake -S . -B build-vs -G "Visual Studio 18 2026" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build build-vs --config Release
```

提交压缩包时一并打包：上述 `.slnx` / `.vcxproj*`、`ImageNodeEditor/` 源码目录、`third_party/`、`CMakeLists.txt`、`README.md`、示例图片与示例 workflow。演示视频和最终报告另行录制/撰写后放入提交材料。

## 已实现

- 节点画布、节点添加、拖动、端口连线、删除。
- 支持选中连线后删除，也支持右键连线删除。
- 属性面板动态编辑参数。
- 预览面板与运行日志。
- workflow JSON 保存和加载。
- DAG 验证与拓扑执行。
- 节点：读入图片、导出图片、预览、裁切、缩放、灰度、亮度/对比度、模糊、旋转/翻转、文字叠加、双图混合、图片拼接。
