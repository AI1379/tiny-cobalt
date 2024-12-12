# Tiny Cobalt

This is a project that implemented a simple C compiler, as an experiment for [Cobalt](https://github.com/The-Cobalt-Develop-Team/cobalt).

## Dependency

We use `xrepo` to manage almost all dependencies, except the `llvm` library. It is required to install `llvm` library locally and set `LLVM_DIR` to `<LLVM_INSTALL_DIR>/lib/cmake/llvm` (for example, `/usr/lib/llvm-18/lib/cmake/llvm`) because we use CMake to find `llvm` package. Thus, it is required to install `cmake` as well.

We use C++20 standard, so make sure your compiler support these new features. For clang, it should be at least 15.0+, and for gcc is 13.0+.

## Development

Open the project using VSCode with following extensions:

- cpptools
- xmake
- clangd

Then set `"xmake.compileCommandsDirectory"` to `build` so that `clangd` can find the `compile_commands.json`.
