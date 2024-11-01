# TCL-OS and DepAnalyzer
A repository housing two advanced system software projects: TCL-OS, an ARM64 operating system with complete hardware support and multi-level privilege modes, and DepAnalyzer, an LLVM-based dependency analysis tool aimed at optimizing code dependencies for nested loops. Each project brings unique contributions to system software development, from low-level OS operations to high-level code analysis and optimization.

## Features
- LLVM-based Dependency Analysis: An LLVM analysis pass designed to analyze code dependencies efficiently.
- Loop Dependency Resolution: Uses Diophantine equations to calculate dependency distances between loop indices.
- Intra-nested Loop Structure Support: Specifically tailored for nested loops, optimizing loop execution and data flow.
- Liveness Analysis: Analyzes variable liveness to solve dependencies amidst C pointer aliasing.

## High-level Design
```
                     +----------------------------------+
                     |            Source Code           |
                     +----------------------------------+
                                       |     
                                       v     
                     +----------------------------------+
                     | LLVM Intermediate Representation |
                     |            (LLVM-IR)             |
                     +----------------------------------+
                                       |     
                  +----------------------------------------+
                  |                                        |
                  v                                        v
+----------------------------------+    +----------------------------------+
|      Dependency Analyzer         |    |        Liveness Analyzer         |
|     (Diophantine Equations)      |    |       (Handles Aliasing)         |
+----------------------------------+    +----------------------------------+
                  |                                        |     
                  v                                        v
+----------------------------------+    +----------------------------------+
|     True/False Dependencies      |    | Reaching Definition of pointer   |
|                                  |    |             analysis             |
+----------------------------------+    +----------------------------------+

```

## Build from Source
First you have to buildup the llvm project.
```
# Ububtu 20.04 as an example:
$ sudo apt-get install build-essential ninja-build
$ wget https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.2/llvm-project-17.0.2.src.tar.xz
$ tar -xf llvm-project-17.0.2.src.tar.xz
$ mkdir build && cd build
$ cmake -G Ninja ../llvm-project-17.0.2.src/llvm \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DCMAKE_BUILD_TYPE=Release
$ ninja
```
To build the project, you can use the following commands:
```
$ cd LoopAnalysis # or PointerReachingDefinition
$ make
```
To use LoopAnalysis, you can run:
```
$ make run1    # First example
$ make run2    # Second example
```
To use PointerReachingDefinition, you can run:
```
$ make run1    # Also 2, 3 and 4 for more examples
```
## Reference
[LLVM GettingStarted](https://llvm.org/docs/GettingStarted.html), 
[Advanced Compilers 2023f HW1 Data Dependency](https://hackmd.io/@z5S_6WvvQNyJk7s0NJxeaQ/rJ_4jcW-p)
[Advanced Compilers 2023f HW2 Pointer Analysis](https://hackmd.io/@z5S_6WvvQNyJk7s0NJxeaQ/HyNFeVhB6)

## License
This project is released under the MIT License. Use of this source code is governed by a MIT License that can be found in the LICENSE file.

