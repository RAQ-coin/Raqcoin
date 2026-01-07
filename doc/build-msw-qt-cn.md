Windows 下 Raqcoin-Qt 编译指南 (Docker 交叉编译)
========================================================================

本指南介绍如何使用基于 Docker 的交叉编译环境构建 Windows 版 Raqcoin-Qt 钱包。这是官方推荐的构建方式，因为它能确保构建环境的一致性，并自动处理所有复杂的依赖关系（如 MXE, Qt, Boost 等）。

前提条件
-------------

*   **Docker**: 请确保您的系统已安装并运行 Docker。

构建步骤
--------------

### 1. 构建构建器镜像 (Builder Image)

首先，您需要构建包含交叉编译工具链的 Docker 镜像。这一步只需要执行一次（除非 `Dockerfile.windows` 发生了更改）。

在项目根目录下打开终端（Windows 下使用 PowerShell，Linux/macOS 下使用 Bash），运行以下命令：

```bash
docker build -t raqcoin-mxe-builder -f Dockerfile.windows .
```

### 2. 编译应用程序

构建器镜像准备好后，您可以使用以下命令编译应用程序。该命令会将您的当前源代码目录挂载到容器中并进行编译。

**Windows (PowerShell):**

```powershell
docker run --rm -v ${PWD}:/app raqcoin-mxe-builder /bin/bash -c "touch raqcoin-qt.pro && x86_64-w64-mingw32.static-qmake-qt5 raqcoin-qt.pro && touch Makefile.Release && make -j4"
```

**Linux / macOS / WSL:**

```bash
docker run --rm -v $(pwd):/app raqcoin-mxe-builder /bin/bash -c "touch raqcoin-qt.pro && x86_64-w64-mingw32.static-qmake-qt5 raqcoin-qt.pro && touch Makefile.Release && make -j4"
```

**注意:** 命令中的 `touch` 操作是为了防止因文件时间戳偏差导致的无限重新配置问题。

### 3. 获取可执行文件

编译成功后，生成的 Windows 可执行文件将位于 `release/` 目录中：

*   `release/raqcoin-qt.exe`

清理构建
-----------

如果您需要清理构建生成的中间文件（如 object 文件、moc 文件等），可以运行：

```bash
docker run --rm -v ${PWD}:/app raqcoin-mxe-builder make clean
```
