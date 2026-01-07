Build instructions for Raqcoin-Qt on Windows (Docker Cross-Compilation)
========================================================================

This guide describes how to build the Raqcoin-Qt wallet for Windows using a Docker-based cross-compilation environment. This approach is recommended as it ensures a consistent build environment and handles all dependencies (MXE, Qt, Boost, etc.) automatically.

Prerequisites
-------------

*   **Docker**: Ensure Docker is installed and running on your system.

Building Steps
--------------

### 1. Build the Builder Image

First, you need to build the Docker image containing the cross-compilation toolchain. This step only needs to be done once (or when `Dockerfile.windows` changes).

Open your terminal (PowerShell on Windows, or Bash on Linux/macOS) in the project root directory and run:

```bash
docker build -t raqcoin-mxe-builder -f Dockerfile.windows .
```

### 2. Compile the Application

Once the builder image is ready, you can compile the application using the following command. This mounts your current source code into the container and compiles it.

**For Windows (PowerShell):**

```powershell
docker run --rm -v ${PWD}:/app raqcoin-mxe-builder /bin/bash -c "touch raqcoin-qt.pro && x86_64-w64-mingw32.static-qmake-qt5 raqcoin-qt.pro && touch Makefile.Release && make -j4"
```

**For Linux / macOS / WSL:**

```bash
docker run --rm -v $(pwd):/app raqcoin-mxe-builder /bin/bash -c "touch raqcoin-qt.pro && x86_64-w64-mingw32.static-qmake-qt5 raqcoin-qt.pro && touch Makefile.Release && make -j4"
```

**Note:** The `touch` commands are used to prevent potential timestamp issues that might cause unnecessary reconfiguration loops.

### 3. Locate the Executable

After the build completes successfully, the compiled Windows executable will be located in the `release/` directory:

*   `release/raqcoin-qt.exe` (or similar)

Cleaning Up
-----------

To clean the build artifacts (object files, moc files, etc.), you can run:

```bash
docker run --rm -v ${PWD}:/app raqcoin-mxe-builder make clean
```
