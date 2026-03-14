# HPC Matrix Multiplication Program

This project is a C++ solution for the CS421 Parallel Programming assignment.

## What The Program Does

The program lets you:

1. create matrix data
2. run serial multiplication
3. run parallel multiplication
4. compare serial and parallel performance

In compare mode, you can enter more than one parallel thread count at once.

Example:

```text
2,4,8
```

This lets the program measure one serial baseline and several parallel cases in the same compare run.

The matrix file used by the program is:

```text
data/matrices.json
```

## Main Files

- `program_controller.h/.cpp`
  Controls the menu, timing, comparison, and CSV output.

- `program_settings.h`
  Stores default values like matrix size, run count, and file paths.

- `create_matrix_data.h/.cpp`
  Creates random test matrices in memory.

- `MatrixFileIO.h/.cpp`
  Saves and loads the JSON matrix file.

- `MatrixMultiplier.h/.cpp`
  Runs serial and OpenMP parallel multiplication.

- `Matrix.h`
  Defines the `Matrix` class.

- `main.cpp`
  Starts the program.

## Generator And Loader

These two parts do different jobs:

- `CreateMatrixData`
  creates new random matrices

- `MatrixFileIO`
  reads matrices from `data/matrices.json` and writes matrices to `data/matrices.json`

So the generator does not load JSON. It only creates data. The file I/O class is the one that reads the JSON file.

## Matrix File Format

The program now saves the matrix file in the same row-by-row JSON style already used in the root `data/matrices.json` file.

Example idea:

```text
{
  "matrixA": [[...], [...], [...]],
  "matrixB": [[...], [...], [...]]
}
```

The loader can still read both:

- the older nested row format
- the newer flat format if such a file already exists

## Matrix Size Input

The program currently supports only square matrices.

You can enter:

- `1000`
- `3x3`

If you enter something like `3x2`, the program will reject it because this project is written for `N x N` matrices.

## Thread Model Used

The parallel version uses **OpenMP**.

You can see this in the code with:

- `#include <omp.h>`
- `#pragma omp parallel for`

This project does **not** use:

- Pthreads
- MPI

## Timing Method

To make timing more stable, the program does not use only one run.

The user chooses how many times to run each case, and the program uses the **average time**.

That means:

- serial time = average of the selected serial runs
- parallel time = average of the selected parallel runs

If you just press Enter, the program uses the default value of **3 runs**.

This gives better results for graphs and presentation.

## Speedup And Efficiency

In compare mode, the program calculates:

- `speedup = serial_time / parallel_time`
- `efficiency = speedup / number_of_threads`

The compare mode also checks that the serial and parallel answers are the same before reporting the result as correct.

## CSV File

The program saves performance data in:

```text
output/performance_results.csv
```

A CSV file is not a graph.

It is a text file with rows and columns. You can open it in:

- Excel
- Google Sheets
- Python

Then use it to create graphs for:

- execution or average time
- speedup
- efficiency

The CSV also stores how many runs were averaged for each row.
New rows also store the individual sample times, so the report can draw a sample-by-sample graph.

This file is the raw performance file from the C++ program.

## Python Chart Tool

This project uses a dependency-based Python tool for graphs.

The Python package used is:

- `plotly` speedup is not perfectly linear because of thread overhead, memory access cost, and coordination cost speedup is not perfectly linear because of thread overhead, memory access cost, and coordination cost
- `kaleido`

The dependency list is stored in:

```text
requirements.txt
```

The project also uses a virtual environment folder:

```text
.venv/
```

Create the virtual environment with:

```bash
make venv
```

Open a shell with the virtual environment activated:

```bash
make activate
```

Install the Python package with:

```bash
./.venv/bin/pip install -r requirements.txt
```

Or let `make charts` do both steps for you.

## Visual Report With Python

```bash
make charts
```

or:

```bash
make graphs
```

or:

```bash
./.venv/bin/python tools/create_charts_and_graphs.py
```

This command creates all chart outputs at once:

- `output/charts/performance_report.html`
- `output/charts/sample_times.html`
- `output/charts/average_time_comparison.html`
- `output/charts/why_speedup_is_not_linear.html`
- `output/charts/summary_chart_data.csv`
- `output/charts/sample_chart_points.csv`
- PNG chart files in `output/charts/png/`
- SVG chart files in `output/charts/svg/`

This chart tool reads:

```text
output/performance_results.csv
```

It creates:

```text
output/charts/performance_report.html
output/charts/sample_times.html
output/charts/average_time_comparison.html
output/charts/why_speedup_is_not_linear.html
```

and also creates separate chart-data CSV files:

```text
output/charts/summary_chart_data.csv
output/charts/sample_chart_points.csv
output/charts/png/
output/charts/svg/
```

Example image files:

```text
output/charts/png/matrix_1000x1000_sample_spread.png
output/charts/png/matrix_1000x1000_average_time.png
output/charts/png/matrix_1000x1000_speedup_gap.png
output/charts/svg/matrix_1000x1000_sample_spread.svg
output/charts/svg/matrix_1000x1000_average_time.svg
output/charts/svg/matrix_1000x1000_speedup_gap.svg
```

Open the HTML file in a browser to see:

- a responsive report header with navigation buttons
- an overview page with the measured results tables
- a separate Sample Times page
- a separate Average Time Comparison page
- a separate Why Speedup Is Not Linear page
- PNG chart files for slides and reports
- SVG chart files for sharp vector export

Speedup and efficiency are still included in the table and chart-data CSV, even though the HTML report now focuses on the clearer graphs only.

The scaling graph is useful for showing that speedup is not perfectly linear.
If the measured line stays below the ideal line, that gap comes from thread overhead, memory access cost, and coordination cost.

Why this is useful:

- `output/performance_results.csv`
  keeps the raw program results

- `output/charts/summary_chart_data.csv`
  keeps the cleaned rows used for average time, speedup, and efficiency charts

- `output/charts/sample_chart_points.csv`
  keeps the sample-by-sample points used for the sample graph

## Python Tool Structure

The Python chart code is split into smaller modules inside:

```text
tools/charts_and_graphs/
```

Main parts:

- `create_charts_and_graphs.py`
  Starts the chart tool.

- `plot_results.py`
  Small wrapper that still runs the chart tool.

- `charts_and_graphs/models.py`
  Stores the data classes used by the tool.

- `charts_and_graphs/csv_loader.py`
  Reads the CSV and groups the latest rows.

- `charts_and_graphs/chart_builder.py`
  Creates the interactive graphs with Plotly.

- `charts_and_graphs/report_builder.py`
  Builds the HTML report.

## Project Structure

```text
HPC-Matrix-Multiplication/
├── requirements.txt
├── Makefile
├── README.md
├── data/
│   └── matrices.json
├── include/
│   ├── create_matrix_data.h
│   ├── json.hpp
│   ├── Matrix.h
│   ├── MatrixFileIO.h
│   ├── MatrixMultiplier.h
│   ├── program_controller.h
│   └── program_settings.h
├── output/
│   └── .gitkeep
├── tools/
│   ├── create_charts_and_graphs.py
│   ├── plot_results.py
│   └── charts_and_graphs/
│       ├── __init__.py
│       ├── chart_builder.py
│       ├── csv_loader.py
│       ├── models.py
│       └── report_builder.py
└── src/
    ├── create_matrix_data.cpp
    ├── main.cpp
    ├── MatrixFileIO.cpp
    ├── MatrixMultiplier.cpp
    └── program_controller.cpp
```

## Build And Run

### Use Makefile On Linux Or WSL

Build the C++ program:

```bash
make
```

Run the Makefile build:

```bash
./build/matrix_program
```

### Use CMake For Cross-Platform Builds

Use this when you want the clean cross-platform build path for Linux, Windows, or other systems with CMake:

Short form:
Use this when you are already in the project root folder.

```bash
cmake -B build-cmake
cmake --build build-cmake
```

Full form:
Use this if you want to be explicit about the source folder.

```bash
cmake -S . -B build-cmake
cmake --build build-cmake
```

Run the CMake build:

```bash
./build-cmake/bin/matrix_program
```

### Python Charts

Create Python venv:

```bash
make venv
```

Activate Python venv shell:

```bash
make activate
```

Create charts:

```bash
make charts
```

## Windows Build

For Windows, use **CMake**, not the `Makefile`.

Recommended way:

Short form:
Use this when you are already inside the project folder in PowerShell or Command Prompt.

```powershell
cmake -B build-cmake
cmake --build build-cmake --config Release
.\build-cmake\bin\matrix_program.exe
```

Full form:
Use this if you want the command to clearly show the source folder.

```powershell
cmake -S . -B build-cmake
cmake --build build-cmake --config Release
.\build-cmake\bin\matrix_program.exe
```

### Windows Python Charts

On Windows, do the Python chart step manually in PowerShell or Command Prompt:

Create the virtual environment:

```powershell
py -m venv .venv
```

Activate the virtual environment:

```powershell
.\.venv\Scripts\activate
```

Install the Python packages:

```powershell
pip install -r requirements.txt
```

Create the HTML report, CSV chart data, PNG charts, and SVG charts:

```powershell
python tools\create_charts_and_graphs.py
```

Notes:

- Visual Studio and MSVC can use OpenMP through CMake.
- MinGW can also work.
- The `Makefile` is mainly for Linux or WSL style environments.
- For plain Windows use the CMake build instead of `make`.
- The Python chart code is cross-platform, but `make activate` is mainly for Linux or WSL shells.
- On Windows, use the manual Python commands above instead of `make charts`.

## Notes For Presentation

Important points you can explain:

- the serial version uses one thread
- the parallel version uses OpenMP threads
- work is divided by rows
- speedup is not perfectly linear because of thread overhead, memory access cost, and coordination cost
- the CSV file is used to make graphs, not as the graph itself
