#include "../include/create_matrix_data.h"

#include <random>

using namespace std;

CreateMatrixData::CreateMatrixData()
    : randomEngine_(random_device{}()),
      distribution_(randomValueMin, randomValueMax)
{
}

pair<Matrix, Matrix> CreateMatrixData::generatePair(size_t size)
{
    Matrix matrixA(size);
    Matrix matrixB(size);

    // This class only creates test data in memory.
    // MatrixFileIO saves it to JSON.
    fill(matrixA);
    fill(matrixB);

    return {move(matrixA), move(matrixB)};
}

void CreateMatrixData::fill(Matrix& matrix)
{
    for (double& value : matrix.values()) {
        value = static_cast<double>(distribution_(randomEngine_));
    }
}
