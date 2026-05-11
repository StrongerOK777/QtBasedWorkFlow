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

## Windows / Visual Studio 提交建议

本项目使用 CMake。Windows 上安装 Qt 6、CMake 和 Visual Studio 后，可以用 CMake 生成 Visual Studio 工程：

```bat
cmake -S . -B build-vs -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2022_64
cmake --build build-vs --config Release
```

课程平台要求提交 Visual Studio 项目压缩包时，建议在 Windows 上生成 `.sln` 后打包：

- `.sln` 或 `.slnx`
- 同名项目目录和 `.vcxproj` / `.vcxproj.filters` / `.vcxproj.user`
- `ImageNodeEditor/` 源码目录、`CMakeLists.txt`、`README.md`
- 示例图片与示例 workflow

演示视频和最终报告需要单独录制/撰写后放入提交材料。

## 已实现

- 节点画布、节点添加、拖动、端口连线、删除。
- 支持选中连线后删除，也支持右键连线删除。
- 属性面板动态编辑参数。
- 预览面板与运行日志。
- workflow JSON 保存和加载。
- DAG 验证与拓扑执行。
- 节点：读入图片、导出图片、预览、裁切、缩放、灰度、亮度/对比度、模糊、旋转/翻转、文字叠加、双图混合、图片拼接。
