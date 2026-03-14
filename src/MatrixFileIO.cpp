#include "../include/MatrixFileIO.h"

#include <cmath>
#include <filesystem>
#include <fstream>

#include "../include/json.hpp"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

bool ensureParentDirectory(const string& filename, string& errorMessage)
{
    const fs::path filePath(filename);
    const fs::path parentDirectory = filePath.parent_path();

    if (parentDirectory.empty()) {
        return true;
    }

    error_code errorCode;
    fs::create_directories(parentDirectory, errorCode);

    if (errorCode) {
        errorMessage = "Unable to create directory: " + parentDirectory.string();
        return false;
    }

    return true;
}

json buildNestedMatrixData(const Matrix& matrix)
{
    json matrixData = json::array();

    for (size_t row = 0; row < matrix.size(); ++row) {
        json rowData = json::array();

        for (size_t column = 0; column < matrix.size(); ++column) {
            rowData.push_back(matrix(row, column));
        }

        matrixData.push_back(move(rowData));
    }

    return matrixData;
}

// Old files and new files use different JSON layouts,
// so the loader accepts both.
bool isNestedMatrix(const json& matrixData)
{
    return matrixData.is_array() && !matrixData.empty() && matrixData.at(0).is_array();
}

bool fillFromNestedArray(const json& matrixData, Matrix& matrix, string& errorMessage)
{
    const size_t matrixSize = matrix.size();

    if (!matrixData.is_array() || matrixData.size() != matrixSize) {
        errorMessage = "Nested matrix data does not match the matrix size.";
        return false;
    }

    for (size_t row = 0; row < matrixSize; ++row) {
        const auto& rowData = matrixData.at(row);

        if (!rowData.is_array() || rowData.size() != matrixSize) {
            errorMessage = "Each matrix row must contain the same number of values.";
            return false;
        }

        for (size_t column = 0; column < matrixSize; ++column) {
            matrix(row, column) = rowData.at(column).get<double>();
        }
    }

    return true;
}

bool fillFromFlatArray(const json& matrixData, Matrix& matrix, string& errorMessage)
{
    const size_t expectedElements = matrix.size() * matrix.size();

    if (!matrixData.is_array() || matrixData.size() != expectedElements) {
        errorMessage = "Flat matrix data does not match the matrix size.";
        return false;
    }

    for (size_t index = 0; index < expectedElements; ++index) {
        matrix.values()[index] = matrixData.at(index).get<double>();
    }

    return true;
}

bool inferMatrixSize(const json& matrixAData, const json& matrixBData, size_t& matrixSize, string& errorMessage)
{
    if (!matrixAData.is_array() || !matrixBData.is_array()) {
        errorMessage = "Matrix data must be stored as arrays.";
        return false;
    }

    const bool nestedA = isNestedMatrix(matrixAData);
    const bool nestedB = isNestedMatrix(matrixBData);

    if (nestedA != nestedB) {
        errorMessage = "Matrix A and Matrix B use different JSON layouts.";
        return false;
    }

    if (nestedA) {
        if (matrixAData.size() != matrixBData.size() || matrixAData.empty()) {
            errorMessage = "Nested matrices do not have matching sizes.";
            return false;
        }

        matrixSize = matrixAData.size();
        return true;
    }

    if (matrixAData.size() != matrixBData.size()) {
        errorMessage = "Matrix A and Matrix B do not have the same number of values.";
        return false;
    }

    const double root = sqrt(static_cast<double>(matrixAData.size()));
    const size_t inferredSize = static_cast<size_t>(root);

    if (inferredSize == 0 || inferredSize * inferredSize != matrixAData.size()) {
        errorMessage = "Unable to infer a square matrix size from the JSON file.";
        return false;
    }

    matrixSize = inferredSize;
    return true;
}

}  // namespace

bool MatrixFileIO::save(const string& filename, const Matrix& matrixA, const Matrix& matrixB, string& errorMessage)
{
    if (matrixA.size() == 0 || matrixB.size() == 0) {
        errorMessage = "Matrices must not be empty.";
        return false;
    }

    if (matrixA.size() != matrixB.size()) {
        errorMessage = "Matrices must have the same dimensions.";
        return false;
    }

    if (!ensureParentDirectory(filename, errorMessage)) {
        return false;
    }

    // Save in the same row-by-row format used by the root data file.
    json document;
    document["matrixA"] = buildNestedMatrixData(matrixA);
    document["matrixB"] = buildNestedMatrixData(matrixB);

    ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        errorMessage = "Unable to open file for writing: " + filename;
        return false;
    }

    outputFile << document.dump();
    return true;
}

bool MatrixFileIO::load(const string& filename, Matrix& matrixA, Matrix& matrixB, string& errorMessage)
{
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        errorMessage = "Unable to open file: " + filename;
        return false;
    }

    try {
        json document;
        inputFile >> document;

        if (!document.contains("matrixA") || !document.contains("matrixB")) {
            errorMessage = "JSON file is missing required fields.";
            return false;
        }

        const auto& matrixAValues = document.at("matrixA");
        const auto& matrixBValues = document.at("matrixB");

        size_t matrixSize = 0;
        // Use the size from the file if it exists.
        // If it does not, work it out from the data.
        if (document.contains("size")) {
            matrixSize = document.at("size").get<size_t>();
        } else if (!inferMatrixSize(matrixAValues, matrixBValues, matrixSize, errorMessage)) {
            return false;
        }

        matrixA = Matrix(matrixSize);
        matrixB = Matrix(matrixSize);

        if (isNestedMatrix(matrixAValues)) {
            if (!fillFromNestedArray(matrixAValues, matrixA, errorMessage) ||
                !fillFromNestedArray(matrixBValues, matrixB, errorMessage)) {
                return false;
            }
        } else {
            if (!fillFromFlatArray(matrixAValues, matrixA, errorMessage) ||
                !fillFromFlatArray(matrixBValues, matrixB, errorMessage)) {
                return false;
            }
        }

        return true;
    } catch (const json::exception& exception) {
        errorMessage = exception.what();
        return false;
    }
}
