<div align="center">

# 🎨 ImageNodeEditor

**A node-based image-processing workflow tool built with Qt 6 + C++17**

Turn read / crop / color / filter / text-overlay / preview / export into visual **nodes**,<br/>wire them on a canvas, tweak parameters, run with one click, and save as JSON.

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![Qt](https://img.shields.io/badge/Qt-6.8-41CD52?logo=qt&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.16%2B-064F8C?logo=cmake&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows%20·%20macOS%20·%20Linux-lightgrey)
![Nodes](https://img.shields.io/badge/nodes-27%2B1-ff7a45)

[简体中文](README.md) ｜ **English**

<img src="docs/images/01-workbench.png" width="820" alt="Main window" />

</div>

---

## Table of Contents

- [✨ Features](#-features)
- [🖼️ Screenshots](#️-screenshots)
- [🧰 Requirements](#-requirements)
- [🔨 Build](#-build)
- [▶️ Run](#️-run)
- [🚀 Quick Start (GUI)](#-quick-start-gui)
- [⌨️ Command Line (picdeal)](#️-command-line-picdeal)
- [📄 Workflow JSON Format](#-workflow-json-format)
- [🗂️ Project Structure](#️-project-structure)
- [⚠️ Known Limitations](#️-known-limitations)
- [📦 Third-Party & License](#-third-party--license)

---

## ✨ Features

- **Visual node canvas**: add / move / delete nodes, drag ports to connect, delete edges, undo / redo.
- **27 built-in nodes + 1 macro node**, in six categories:

  | Category | Nodes |
  | --- | --- |
  | Input / Output | Image Input, Image Output, Preview |
  | Geometry | Crop, Resize, Rotate/Flip, Grid Split |
  | Color | Grayscale, Brightness/Contrast, Invert, Threshold, Hue/Saturation, Channel Split/Merge |
  | Filter | Blur, Sharpen, Edge Detect, Pixelate |
  | Compositing | Text Overlay, Blend, Image Merge, Mask Blend, Watermark |
  | Batch | Folder Input, List Pick, List Merge, List Export |

- **Execution engine**: validates port direction & type on connect; runs a DAG check and topological sort before execution; returns clear error messages instead of crashing.
- **Per-node output thumbnails**: after a run, each node card shows its output image, refreshed live as you edit parameters.
- **Workflow JSON**: save / load the "recipe" (nodes, positions, parameters, edges); supports relative and non-ASCII paths.
- **Checkpoints & diff**: save workflow checkpoints and compare any two (or the current canvas) side by side — output thumbnails plus a structural diff.
- **Smart connect suggestions**: drop a dragged wire on empty canvas to get type-compatible node suggestions; pick one and it's created and wired automatically.
- **`picdeal` CLI**: run workflows headless, with an ffmpeg-style linear pipeline, folder batch processing, and a git-like save history.

## 🖼️ Screenshots

| Build a workflow (wires + parameters) | After running — output thumbnails |
| :---: | :---: |
| <img src="docs/images/02-workflow.png" width="400" alt="Wires and parameters" /> | <img src="docs/images/03-executed.png" width="400" alt="Execution result" /> |

---

## 🧰 Requirements

| Dependency | Version / Notes |
| --- | --- |
| C++ compiler | C++17 (MSVC 2022/2026, Clang, or GCC) |
| Qt | **6.5+** (developed & verified on **6.8.3**); components `Core Gui Widgets Qml Quick QuickControls2 QuickWidgets`, plus `qtshadertools` on Windows |
| CMake | 3.16+ (for the cross-platform build) |
| Visual Studio | 2022 / 2026 (Windows only, for the VS project) |

> On Windows, install the Qt `msvc2022_64` kit via `aqtinstall` or the official installer; the default path used here is `C:\Qt\6.8.3\msvc2022_64`.

---

## 🔨 Build

Two equivalent build paths — **pick one**; both compile the same source.

<details open>
<summary><b>Option 1 — Visual Studio (Windows, easiest)</b></summary>

The repo ships a native VS solution compiled directly by MSVC; no command line needed.

1. Install Qt 6.8.3 `msvc2022_64` (default `C:\Qt\6.8.3\msvc2022_64`) and Visual Studio (with the "Desktop development with C++" workload).
2. Double-click **`ImageNodeEditor.slnx`** in the repo root.
3. Press **F5** (Release | x64) to build and run.

Output lands in `out/Release/`, with Qt runtime and QML modules already deployed by `windeployqt` — ready to run or copy elsewhere.

> If Qt is installed elsewhere, set the `QTDIR` environment variable or edit `QtDir` in the `.vcxproj`.
> After editing a `Q_OBJECT` header or a `qml/` resource, run `ImageNodeEditor/GeneratedFiles/regen.bat` to refresh the pre-generated moc/rcc, then rebuild.

</details>

<details>
<summary><b>Option 2 — CMake (cross-platform)</b></summary>

**Windows** (generates a VS solution):

```bat
cmake -S . -B build-vs -G "Visual Studio 18 2026" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build build-vs --config Release
```

Output in `build-vs\Release\`; you can also open `build-vs\ImageNodeEditor.sln` and press F5.

**macOS**:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt)
cmake --build build -j
```

Produces `build/ImageNodeEditor.app`.

**Linux**:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.8.3/gcc_64
cmake --build build -j
```

> The CMake path handles moc/rcc automatically (AUTOMOC/AUTORCC) and builds QtNodes from source — no manual regen needed.

</details>

---

## ▶️ Run

**Graphical interface (GUI)**:

```bash
# Windows (VS build)
out\Release\ImageNodeEditor.exe
# Windows (CMake build)
build-vs\Release\ImageNodeEditor.exe
# macOS
open build/ImageNodeEditor.app
```

**Quick check with the sample workflow** (headless, no window):

```bash
# Windows
out\Release\ImageNodeEditor.exe run ImageNodeEditor\resources\workflows\sample.json
# macOS / Linux
./build/ImageNodeEditor.app/Contents/MacOS/ImageNodeEditor run ImageNodeEditor/resources/workflows/sample.json
```

---

## 🚀 Quick Start (GUI)

1. Launch the app; in the left **Node Library**, drag **Image Input** onto the canvas.
2. Double-click its "file path" parameter and pick an image.
3. Drag in more nodes such as **Grayscale**, **Blur**, **Preview**.
4. Drag from a node's **output port** to the next node's **input port** to connect (dropping on empty canvas pops up compatible-node suggestions).
5. Edit parameters right on the node card.
6. Click **Run** in the toolbar (or open the command palette with `Ctrl+Shift+P` and search "Run"). Each node shows its output thumbnail; the Preview node sends its result to the preview pane.
7. **File → Save** to store a `workflow.json`; **Open** it later to continue.

---

## ⌨️ Command Line (picdeal)

A convenient `picdeal` (`picdeal.exe` on Windows — the same binary as the app) is produced next to the build output. You can also call the main exe with a subcommand.

```text
picdeal pipe   -i INPUT [--op [key=val|val]...] -o OUTPUT   # ffmpeg-style linear pipeline, runs it
picdeal batch  -d INPUT_DIR -o OUTPUT_DIR [--op ...]        # apply one pipeline to every image in a folder
picdeal build  -i ... [--op ...] --save workflow.json       # build and save a workflow (openable in the GUI)
picdeal run    workflow.json                                # run a saved workflow
picdeal validate workflow.json                              # validate only
picdeal nodes                                               # list all nodes and parameters
picdeal save   workflow.json [-m NOTE]                      # store into the save history (visible in GUI)
picdeal log    [--timeline]                                 # git-log-style list of the save history
picdeal restore <id> [-o out.json]                          # roll back to a saved revision
picdeal help | version
```

Common `--op` operations: `grayscale`, `blur [radius]`, `resize WxH`, `crop x= y= width= height=`, `brightness VAL`, `rotate ANGLE`, `text TEXT`, `sharpen`, `edge`, `invert`, `threshold`, `hsl`, `pixelate`, `blend SECOND_IMG`, `merge ...`, `watermark MARK_IMG`, `preview` (see `picdeal help` for the full list).

```bash
picdeal pipe -i in.png --grayscale --blur 3 --resize 800x600 -o out.png
picdeal pipe -i a.png --blend b.png opacity=0.4 -o merged.png
picdeal batch -d photos -o out --grayscale --resize 1024x768 --format png
picdeal nodes
```

> Legacy entry point still works: `ImageNodeEditor.exe --no-gui --workflow path/to/workflow.json`.

---

## 📄 Workflow JSON Format

The file stores the "recipe" only — no image binary:

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

- Image paths are stored as relative paths when possible, resolved against the workflow file's directory on load.
- Node type names (`type`) are listed by `picdeal nodes`; port names are stable (e.g. `image` / `output`).

---

## 🗂️ Project Structure

```text
ImageNodeEditor.slnx              VS solution (for submission, Windows)
CMakeLists.txt                    cross-platform build entry
ImageNodeEditor/
  main.cpp                        entry point: GUI vs. command-line dispatch
  app/                            picdeal subcommand dispatch
  core/                           basic types (port type, edge, parameter, result)
  nodes/                          ImageNode polymorphic base + NodeFactory + all nodes
  processing/                     pure image algorithms (crop/resize/color/filter/compose)
  workflow/                       graph, JSON serialization, validation, execution, save history
  gui/                            main window, canvas, parameter panel, preview, diff dialog
  qml/                            QML workbench shell (title bar / activity bar / sidebar)
  util/                           path utilities
  resources/                      sample images and workflows
third_party/                      QtNodes canvas library, Codicons icon font
docs/images/                      screenshots for README / report
```

UI, data model, image algorithms, and the execution engine are kept separate; the GUI and CLI both funnel into the same `workflow/` + `nodes/`. See [`struct.md`](struct.md) for detailed responsibilities and [`solution.md`](solution.md) for the overall design.

---

## ⚠️ Known Limitations

- The native VS project is configured for `Release | x64` only (Debug needs a debug Qt build and an extra configuration).
- To bound memory and time, there are caps on very large images, oversized blur radius, etc.
- CLI mode renders offscreen and needs the `qoffscreen` platform plugin (auto-deployed by the build; include it if you copy the run directory by hand).
- All cross-platform code goes through Qt APIs with no platform-specific system calls; the macOS native window chrome is handled separately in `gui/NativeWindowChrome_mac.mm`.

## 📦 Third-Party & License

- [QtNodes](https://github.com/paceholder/nodeeditor): node canvas library, vendored under `third_party/QtNodes/`; see that directory for its license.
- [Codicons](https://github.com/microsoft/vscode-codicons): icon font (CC-BY-4.0), under `third_party/codicons/`.
- This project is coursework; its core features rely only on built-in Qt capabilities (`QImage` / `QPainter`, etc.) with no heavyweight dependencies such as OpenCV.
