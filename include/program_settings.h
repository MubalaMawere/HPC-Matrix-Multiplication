#ifndef PROGRAM_SETTINGS_H
#define PROGRAM_SETTINGS_H

#include <cstddef>

inline constexpr std::size_t defaultMatrixSize = 1000;
inline constexpr int defaultRunCount = 3;
inline constexpr int randomValueMin = 1;
inline constexpr int randomValueMax = 100;
inline constexpr double comparisonTolerance = 1e-9;

inline constexpr char matrixDataPath[] = "data/matrices.json";
inline constexpr char resultsCsvPath[] = "output/performance_results.csv";

#endif
