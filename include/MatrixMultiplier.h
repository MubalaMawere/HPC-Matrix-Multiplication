#ifndef MATRIX_MULTIPLIER_H
#define MATRIX_MULTIPLIER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Matrix.h"

// Stores how many multiply and add operations were done.
struct OperationCounts {
    std::uint64_t multiplications = 0;
    std::uint64_t additions = 0;

    // Returns the total number of operations.
    std::uint64_t total() const noexcept
    {
        return multiplications + additions;
    }
};

// Stores the results after one measured test.
struct PerformanceResult {
    std::string mode;
    std::size_t matrixSize = 0;
    int threadsUsed = 1;
    // Shows how many runs were averaged to get the time.
    int sampleCount = 1;
    // Stores the time of each individual sample run.
    std::vector<double> sampleTimes;
    double executionSeconds = 0.0;
    double speedup = 0.0;
    double efficiency = 0.0;
    bool hasComparisonData = false;
    bool hasVerification = false;
    bool verified = false;
    OperationCounts operations;
};

// Runs serial and parallel matrix multiplication.
// The parallel version uses OpenMP threads.
class MatrixMultiplier {
public:
    // Multiplies the matrices using one thread.
    static Matrix multiplySerial(const Matrix& matrixA, const Matrix& matrixB);

    // Multiplies the matrices using the given number of threads.
    static Matrix multiplyParallel(const Matrix& matrixA, const Matrix& matrixB, int threadCount);

    // Checks if two result matrices are the same.
    static bool areEqual(const Matrix& left, const Matrix& right, double tolerance);

    // Calculates the expected number of operations for an N x N matrix.
    static OperationCounts calculateOperationCounts(std::size_t matrixSize);

private:
    static Matrix multiply(const Matrix& matrixA, const Matrix& matrixB, int threadCount, bool parallel);
    static void validateInputs(const Matrix& matrixA, const Matrix& matrixB);
};

#endif
