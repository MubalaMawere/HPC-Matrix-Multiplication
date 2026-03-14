#include "../include/MatrixMultiplier.h"

#include <cmath>
#include <stdexcept>

#include <omp.h>

using namespace std;

// This file uses OpenMP for the parallel version.
Matrix MatrixMultiplier::multiplySerial(const Matrix& matrixA, const Matrix& matrixB)
{
    return multiply(matrixA, matrixB, 1, false);
}

Matrix MatrixMultiplier::multiplyParallel(const Matrix& matrixA, const Matrix& matrixB, int threadCount)
{
    if (threadCount < 1) {
        threadCount = 1;
    }

    return multiply(matrixA, matrixB, threadCount, true);
}

bool MatrixMultiplier::areEqual(const Matrix& left, const Matrix& right, double tolerance)
{
    if (left.size() != right.size()) {
        return false;
    }

    const auto& leftValues = left.values();
    const auto& rightValues = right.values();

    for (size_t index = 0; index < leftValues.size(); ++index) {
        if (fabs(leftValues[index] - rightValues[index]) > tolerance) {
            return false;
        }
    }

    return true;
}

OperationCounts MatrixMultiplier::calculateOperationCounts(size_t matrixSize)
{
    OperationCounts counts;
    counts.multiplications = static_cast<uint64_t>(matrixSize) * matrixSize * matrixSize;
    counts.additions = static_cast<uint64_t>(matrixSize) * matrixSize * (matrixSize - 1);
    return counts;
}

Matrix MatrixMultiplier::multiply(const Matrix& matrixA, const Matrix& matrixB, int threadCount, bool parallel)
{
    validateInputs(matrixA, matrixB);

    Matrix result(matrixA.size());
    const size_t size = matrixA.size();
    const auto& leftValues = matrixA.values();
    const auto& rightValues = matrixB.values();
    auto& outputValues = result.values();
    const int loopSize = static_cast<int>(size);

    // OpenMP splits the result rows across threads.
    // Each thread works on different rows, so they do not write to the same cells.
#pragma omp parallel for if (parallel) num_threads(threadCount) schedule(static)
    for (int row = 0; row < loopSize; ++row) {
        const size_t rowOffset = static_cast<size_t>(row) * size;

        // This loop order reads and writes rows in a simple way.
        for (size_t k = 0; k < size; ++k) {
            const double leftValue = leftValues[rowOffset + k];
            const size_t rightRowOffset = k * size;

            for (size_t column = 0; column < size; ++column) {
                outputValues[rowOffset + column] += leftValue * rightValues[rightRowOffset + column];
            }
        }
    }

    return result;
}

void MatrixMultiplier::validateInputs(const Matrix& matrixA, const Matrix& matrixB)
{
    if (matrixA.size() == 0 || matrixB.size() == 0) {
        throw invalid_argument("Matrices must not be empty.");
    }

    if (matrixA.size() != matrixB.size()) {
        throw invalid_argument("Matrix multiplication requires square matrices of the same size.");
    }
}
