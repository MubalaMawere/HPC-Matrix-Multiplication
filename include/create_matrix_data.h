#ifndef CREATE_MATRIX_DATA_H
#define CREATE_MATRIX_DATA_H

#include <cstddef>
#include <random>
#include <utility>

#include "program_settings.h"
#include "Matrix.h"

class CreateMatrixData {
public:
    CreateMatrixData();
    std::pair<Matrix, Matrix> generatePair(std::size_t size);

private:
    void fill(Matrix& matrix);

    std::mt19937 randomEngine_;
    std::uniform_int_distribution<int> distribution_;
};

#endif
