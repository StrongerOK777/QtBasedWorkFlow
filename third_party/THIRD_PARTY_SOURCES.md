# GUI Third-Party Sources

This file records the vendored GUI dependency used by the main application.
The upstream `.git` directory is not vendored. The original license and README
remain in the dependency root.

| Directory | Upstream | Snapshot commit | License file kept |
| --- | --- | --- | --- |
| `QtNodes/` | <https://github.com/paceholder/nodeeditor> | `bcb0b35f21a0cbec95dd1e5f7dc6a9a7a7530252` | `QtNodes/LICENSE.rst` |

Icon policy:

- The VS Code-style workbench icons used by the QML shell are small project-owned
  vector drawings in `ImageNodeEditor/qml/WorkbenchIcon.qml`.
- They follow common editor UI metaphors and avoid Microsoft/VS Code branding.
- No Codicons font file or upstream SVG asset is currently vendored.

Import policy:

- keep the linked third-party boundary explicit in top-level CMake;
- do not fetch them from the network during build;
- update this table whenever a snapshot is replaced;
- prune upstream examples, tests and docs that are not built by this repository.
