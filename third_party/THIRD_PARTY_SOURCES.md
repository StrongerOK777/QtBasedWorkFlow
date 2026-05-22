# GUI Third-Party Sources

This file records the vendored GUI dependency used by the main application.
The upstream `.git` directory is not vendored. The original license and README
remain in the dependency root.

| Directory | Upstream | Snapshot commit | License file kept |
| --- | --- | --- | --- |
| `QtNodes/` | <https://github.com/paceholder/nodeeditor> | `bcb0b35f21a0cbec95dd1e5f7dc6a9a7a7530252` | `QtNodes/LICENSE.rst` |

Import policy:

- keep the linked third-party boundary explicit in top-level CMake;
- do not fetch them from the network during build;
- update this table whenever a snapshot is replaced;
- prune upstream examples, tests and docs that are not built by this repository.
