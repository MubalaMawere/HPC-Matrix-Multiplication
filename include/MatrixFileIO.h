#ifndef MATRIX_FILE_IO_H
#define MATRIX_FILE_IO_H

#include <string>

#include "Matrix.h"

// This class saves matrices to JSON and loads matrices from JSON.
class MatrixFileIO {
public:
    // Saves matrix A and matrix B to the given file.
    static bool save(const std::string& filename, const Matrix& matrixA, const Matrix& matrixB, std::string& errorMessage);

    // Loads matrix A and matrix B from the given file.
    static bool load(const std::string& filename, Matrix& matrixA, Matrix& matrixB, std::string& errorMessage);
};

#endif
