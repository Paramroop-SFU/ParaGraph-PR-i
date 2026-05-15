# ParaGraph-PR 🚀

A high-performance, multi-threaded implementation of the **PageRank algorithm** written in C++. This project focuses on parallelizing graph traversal and ranking using a push-based approach, synchronization primitives, and configurable data types.

## 🌟 Features

*   **Multi-threaded Execution:** Uses `std::thread` to distribute graph processing across multiple CPU cores.
*   **Custom Synchronization:** Implements thread safety using `std::mutex` and a `CustomBarrier` to coordinate iterations.
*   **Hybrid Data Support:** Supports both `int64_t` and `float` precision through preprocessor directives (`USE_INT`).
*   **Scalable Architecture:** Built to handle large-scale graphs with configurable worker counts and iteration limits.
*   **Comprehensive Toolset:** Includes additional parallel implementations for Triangle Counting and Pi calculation.

---

## 🛠 Prerequisites

*   A **C++11** (or higher) compatible compiler (e.g., `g++`, `clang`).
*   `make` for building the project.
*   A **Linux-based environment** (recommended for performance testing).

---

## 📂 Project Structure

```text
ASSIGNMENT1/
├── core/
│   ├── graph.h                   # Graph data structures
│   ├── utils.h                   # Timer and utility functions
│   └── cxxopts.h                 # Command-line option parsing
├── page_rank_parallel.cpp         # Main parallel PageRank logic
├── triangle_counting_parallel.cpp # Parallel Triangle Counting
├── pi_calculation_parallel.cpp    # Parallel Pi calculation
└── Makefile                      # Build automation
