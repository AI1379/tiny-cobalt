# Tiny Cobalt

This is a project that implemented a compiler of a simple C-like language, as an experiment for [Cobalt](https://github.com/The-Cobalt-Develop-Team/cobalt).

## Dependency

We use `xrepo` to manage almost all dependencies, except the `llvm` library. It is required to install `llvm` library locally and set `LLVM_DIR` to `<LLVM_INSTALL_DIR>/lib/cmake/llvm` (for example, `/usr/lib/llvm-19/lib/cmake/llvm`) because we use CMake to find `llvm` package. Thus, it is required to install `cmake` as well.

We use C++20 standard, so make sure your compiler support these new features. Features will be tested on gcc-14 and clang-19. MSVC platform has not been tested.

We use Flex and Bison to generate lexer and parser, so please make sure that you have installed them in your system.

## Development

Open the project using VSCode with following extensions:

- cpptools
- xmake
- clangd

Then set `"xmake.compileCommandsDirectory"` to `build` so that `clangd` can find the `compile_commands.json`.

[Development Log](https://www.listener1379.top/2025/02/tiny-cobalt-dev/)
