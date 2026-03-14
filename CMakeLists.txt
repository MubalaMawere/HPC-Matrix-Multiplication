cmake_minimum_required(VERSION 3.16)

project(HPCMatrixMultiplication LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP REQUIRED COMPONENTS CXX)

add_executable(matrix_program
    src/main.cpp
    src/MatrixFileIO.cpp
    src/MatrixMultiplier.cpp
    src/create_matrix_data.cpp
    src/program_controller.cpp
)

target_include_directories(matrix_program PRIVATE include)
target_link_libraries(matrix_program PRIVATE OpenMP::OpenMP_CXX)

if (MSVC)
    target_compile_options(matrix_program PRIVATE /W4)
else()
    target_compile_options(matrix_program PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(matrix_program PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin"
)
