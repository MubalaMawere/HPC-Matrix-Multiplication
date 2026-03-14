#ifndef MATRIX_H
#define MATRIX_H

#include <cstddef>
#include <vector>

class Matrix {
public:
    // The size is given when the matrix object is created.
    explicit Matrix(std::size_t size = 0)
        : size_(size),
          values_(size * size, 0.0)
    {
    }

    std::size_t size() const noexcept
    {
        return size_;
    }

    bool empty() const noexcept
    {
        return values_.empty();
    }

    // This class stores an N x N matrix.
    // Example for a 3x3 matrix:
    // [ a b c ]
    // [ d e f ]  ->  [a, b, c, d, e, f, g, h, i]
    // [ g h i ]
    double& operator()(std::size_t row, std::size_t column)
    {
        return values_[toIndex(row, column)];
    }

    double operator()(std::size_t row, std::size_t column) const
    {
        return values_[toIndex(row, column)];
    }

    std::vector<double>& values() noexcept
    {
        return values_;
    }

    const std::vector<double>& values() const noexcept
    {
        return values_;
    }

private:
    // size_ is used to change row and column into one vector position.
    std::size_t toIndex(std::size_t row, std::size_t column) const noexcept
    {
        return row * size_ + column;
    }

    // If size_ is 1000, then the matrix is 1000 x 1000.
    std::size_t size_ = 0;
    std::vector<double> values_;
};

#endif
