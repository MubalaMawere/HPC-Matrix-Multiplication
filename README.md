# HPC Matrix Multiplication – Serial Implementation

## Project Overview

This project implements a **serial matrix multiplication benchmark** for a **1000 × 1000 matrix** using C++. The goal is to establish a **baseline performance measurement** that will later be compared with a **parallel implementation using OpenMP**.

Two matrices (**A** and **B**) are generated automatically and stored in a JSON file. The serial program loads these matrices, performs multiplication, measures execution time, and reports the number of arithmetic operations performed.

This project demonstrates key concepts from **High Performance Computing (HPC)** including:

* Algorithmic complexity (O(N^3))
* Memory-efficient matrix storage
* Cache-friendly multiplication order
* Performance benchmarking
* Operation counting

---

# Project Structure

```
HPC-Matrix-Multiplication
│
├── data
│   └── matrices.json
│
├── src
│   ├── main.cpp
│   ├── matrix_loader.cpp
│   ├── matrix_multiply.cpp
│   ├── matrix_generator.cpp
│   └── timer.cpp
│
├── include
│   ├── constants.h
│   ├── json.hpp
│   ├── matrix_loader.h
│   ├── matrix_multiply.h
│   └── timer.h
│
├── results
│   └── execution_times.csv
│
└── build
```

---

# Program Components

## 1. Matrix Generator

Generates two **1000 × 1000 matrices** filled with random numbers and saves them in:

```
data/matrices.json
```

Each matrix contains:

```
1,000,000 numbers
```

Total numbers generated:

```
2,000,000
```

---

## 2. Serial Matrix Multiplication

The program loads matrices **A** and **B** from the JSON file and computes:

```
C = A × B
```

using the matrix multiplication formula:

```
C[i][j] = Σ (A[i][k] × B[k][j])
```

---

## 3. Execution Time Measurement

The program measures the execution time using C++:

```
chrono high_resolution_clock
```

The time is displayed and also stored for analysis.

---

## 4. Operation Counting

The program reports the theoretical number of arithmetic operations.

For matrix size:

```
N = 1000
```

Multiplications:

```
N³ = 1,000,000,000
```

Additions:

```
N²(N−1) = 999,000,000
```

Total operations:

```
1,999,000,000
```

---

# How to Compile and Run

All commands should be executed from the **project root directory**.

Example:

```
HPC-Matrix-Multiplication
```

---

# Step 1 – Compile the Matrix Generator

```
g++ src/matrix_generator.cpp -Iinclude -o build/matrix_generator
```

---

# Step 2 – Run the Matrix Generator

```
.\build\matrix_generator
```

This will generate:

```
data/matrices.json
```

containing matrices **A** and **B**.

---

# Step 3 – Compile the Serial Matrix Program

```
g++ src/main.cpp src/matrix_loader.cpp src/matrix_multiply.cpp src/timer.cpp -Iinclude -o build/serial_matrix
```

---

# Step 4 – Run the Serial Program

```
.\build\serial_matrix
```

---

# Example Program Output

```
=====================================
Serial Matrix Multiplication (HPC)
Matrix Size: 1000 x 1000
=====================================

Loading matrices from JSON...
Matrices loaded successfully.

Starting serial multiplication...

Execution Time: 8.41 seconds

Operation Statistics
----------------------
Multiplications : 1000000000
Additions       : 999000000
Total Operations: 1999000000

Results saved to results/execution_times.csv
```

---

# Results Output

Execution times are stored in:

```
results/execution_times.csv
```

Example format:

```
MatrixSize,ExecutionTime
1000,8.41
```

This data can later be used to generate **algorithmic performance graphs**.

---

# Algorithm Complexity

Matrix multiplication has time complexity:

```
O(N³)
```

For:

```
N = 1000
```

Total loop iterations:

```
1000 × 1000 × 1000 = 1,000,000,000
```

This large computational workload makes the problem suitable for **parallel performance comparison**.

---

# Future Work

This serial implementation will serve as a **baseline** for comparison with a **parallel implementation using OpenMP**. Performance metrics such as **speedup** and **efficiency** will be calculated.

Speedup:

```
Speedup = Serial Time / Parallel Time
```

Efficiency:

```
Efficiency = Speedup / Number of Threads
```

---

# Author
Author: Mubala Mawere
CS421 – High Performance Computing
Serial Matrix Multiplication Benchmark


