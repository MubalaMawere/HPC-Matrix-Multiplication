#include "../include/program_controller.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <omp.h>

#include "../include/program_settings.h"
#include "../include/create_matrix_data.h"
#include "../include/MatrixFileIO.h"
#include "../include/MatrixMultiplier.h"

using namespace std;
namespace fs = std::filesystem;

namespace {

struct RunExecution {
    Matrix result;
    PerformanceResult resultInfo;
};

struct TimedSamples {
    double averageSeconds = 0.0;
    vector<double> sampleTimes;
};

constexpr const char* currentCsvHeader =
    "mode,matrix_size,threads,sample_count,execution_seconds,speedup,efficiency,verified,multiplications,additions,total_operations,sample_times";
constexpr const char* previousCsvHeader =
    "mode,matrix_size,threads,sample_count,execution_seconds,speedup,efficiency,verified,multiplications,additions,total_operations";
constexpr const char* oldCsvHeader =
    "mode,matrix_size,threads,execution_seconds,speedup,efficiency,verified,multiplications,additions,total_operations";

class Stopwatch {
public:
    void start()
    {
        startTime_ = chrono::high_resolution_clock::now();
    }

    void stop()
    {
        endTime_ = chrono::high_resolution_clock::now();
    }

    double elapsedSeconds() const
    {
        const chrono::duration<double> duration = endTime_ - startTime_;
        return duration.count();
    }

private:
    chrono::high_resolution_clock::time_point startTime_{};
    chrono::high_resolution_clock::time_point endTime_{};
};

template <typename Operation>
TimedSamples measureAverageTime(Operation operation, Matrix& result, int sampleCount)
{
    TimedSamples measuredSamples;
    measuredSamples.sampleTimes.reserve(static_cast<size_t>(sampleCount));

    for (int run = 0; run < sampleCount; ++run) {
        Stopwatch stopwatch;
        stopwatch.start();

        Matrix currentResult = operation();

        stopwatch.stop();
        const double elapsedSeconds = stopwatch.elapsedSeconds();
        measuredSamples.sampleTimes.push_back(elapsedSeconds);
        measuredSamples.averageSeconds += elapsedSeconds;
        result = move(currentResult);
    }

    measuredSamples.averageSeconds /= static_cast<double>(sampleCount);
    return measuredSamples;
}

void printOptionalMetric(const char* label, double value, bool available)
{
    cout << label;
    if (available) {
        cout << fixed << setprecision(4) << value;
    } else {
        cout << "N/A";
    }
    cout << '\n';
}

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

vector<string> splitSimpleCsvLine(const string& line)
{
    vector<string> fields;
    string currentField;

    for (char character : line) {
        if (character == ',') {
            fields.push_back(currentField);
            currentField.clear();
        } else {
            currentField.push_back(character);
        }
    }

    fields.push_back(currentField);
    return fields;
}

string joinSimpleCsvLine(const vector<string>& fields)
{
    string line;

    for (size_t index = 0; index < fields.size(); ++index) {
        if (index > 0) {
            line += ',';
        }

        line += fields[index];
    }

    return line;
}

bool upgradeResultsCsvIfNeeded(const string& filename, string& errorMessage)
{
    if (!fs::exists(filename) || fs::is_empty(filename)) {
        return true;
    }

    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        errorMessage = "Unable to open CSV file: " + filename;
        return false;
    }

    string header;
    getline(inputFile, header);

    if (header == currentCsvHeader) {
        return true;
    }

    enum class CsvFormat {
        Previous,
        Old
    };

    CsvFormat csvFormat{};

    if (header == previousCsvHeader) {
        csvFormat = CsvFormat::Previous;
    } else if (header == oldCsvHeader) {
        csvFormat = CsvFormat::Old;
    } else {
        errorMessage = "CSV file has an unknown format: " + filename;
        return false;
    }

    vector<string> upgradedLines;
    string line;

    while (getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }

        vector<string> fields = splitSimpleCsvLine(line);

        if (csvFormat == CsvFormat::Previous) {
            if (fields.size() != 11) {
                errorMessage = "CSV file has a row with the wrong number of columns: " + filename;
                return false;
            }

            fields.push_back("");
            upgradedLines.push_back(joinSimpleCsvLine(fields));
            continue;
        }

        if (fields.size() != 10) {
            errorMessage = "CSV file has a row with the wrong number of columns: " + filename;
            return false;
        }

        // Old rows were always averaged with the old default value of 3 runs.
        fields.insert(fields.begin() + 3, to_string(defaultRunCount));
        fields.push_back("");
        upgradedLines.push_back(joinSimpleCsvLine(fields));
    }

    ofstream outputFile(filename, ios::trunc);
    if (!outputFile.is_open()) {
        errorMessage = "Unable to rewrite CSV file: " + filename;
        return false;
    }

    outputFile << currentCsvHeader << '\n';
    for (const string& upgradedLine : upgradedLines) {
        outputFile << upgradedLine << '\n';
    }

    return true;
}

string serializeSampleTimes(const vector<double>& sampleTimes)
{
    ostringstream output;
    output << fixed << setprecision(6);

    for (size_t index = 0; index < sampleTimes.size(); ++index) {
        if (index > 0) {
            output << ';';
        }

        output << sampleTimes[index];
    }

    return output.str();
}

void printResult(const PerformanceResult& result)
{
    cout << "\n---------------------------------------------\n";
    cout << result.mode << " run\n";
    cout << "Matrix size      : " << result.matrixSize << " x " << result.matrixSize << '\n';
    cout << "Threads used     : " << result.threadsUsed << '\n';
    cout << "Runs averaged    : " << result.sampleCount << '\n';
    cout << "Average time     : " << fixed << setprecision(6)
         << result.executionSeconds << " seconds (" << result.sampleCount << " runs)\n";
    printOptionalMetric("Speedup          : ", result.speedup, result.hasComparisonData);
    printOptionalMetric("Efficiency       : ", result.efficiency, result.hasComparisonData);
    cout << "Multiplications  : " << result.operations.multiplications << '\n';
    cout << "Additions        : " << result.operations.additions << '\n';
    cout << "Total operations : " << result.operations.total() << '\n';
    cout << "Verification     : ";
    if (result.hasVerification) {
        cout << (result.verified ? "Passed" : "Failed");
    } else {
        cout << "Not run";
    }
    cout << '\n';
}

void printComparison(const PerformanceResult& serialResult, const PerformanceResult& parallelResult)
{
    cout << "\n=============================================\n";
    cout << "Comparison summary for Parallel (" << parallelResult.threadsUsed << "T)\n";
    cout << "Runs averaged    : " << serialResult.sampleCount << '\n';
    cout << "Serial avg time  : " << fixed << setprecision(6)
         << serialResult.executionSeconds << " seconds\n";
    cout << "Parallel avg time: " << parallelResult.executionSeconds << " seconds\n";
    cout << "Speedup          : " << setprecision(4) << parallelResult.speedup << '\n';
    cout << "Efficiency       : " << parallelResult.efficiency << '\n';
    cout << "Correctness check: " << (parallelResult.verified ? "Passed" : "Failed") << '\n';
}

bool appendCsv(const string& filename, const PerformanceResult& result, string& errorMessage)
{
    if (!ensureParentDirectory(filename, errorMessage)) {
        return false;
    }

    if (!upgradeResultsCsvIfNeeded(filename, errorMessage)) {
        return false;
    }

    // Write the column names only once.
    const bool needsHeader = !fs::exists(filename) || fs::is_empty(filename);
    ofstream outputFile(filename, ios::app);

    if (!outputFile.is_open()) {
        errorMessage = "Unable to open CSV file: " + filename;
        return false;
    }

    if (needsHeader) {
        outputFile << currentCsvHeader << '\n';
    }

    outputFile << result.mode << ','
               << result.matrixSize << ','
               << result.threadsUsed << ','
               << result.sampleCount << ','
               << fixed << setprecision(6) << result.executionSeconds << ',';

    if (result.hasComparisonData) {
        outputFile << setprecision(6) << result.speedup << ','
                   << result.efficiency << ',';
    } else {
        outputFile << ','
                   << ',';
    }

    if (result.hasVerification) {
        outputFile << (result.verified ? "true" : "false");
    }

    outputFile << ','
               << result.operations.multiplications << ','
               << result.operations.additions << ','
               << result.operations.total() << ','
               << serializeSampleTimes(result.sampleTimes) << '\n';

    return true;
}

bool parsePositiveSize(const string& text, size_t& value)
{
    if (text.empty()) {
        return false;
    }

    size_t processed = 0;

    try {
        const unsigned long long parsedValue = stoull(text, &processed);
        if (processed != text.size() || parsedValue == 0) {
            return false;
        }

        value = static_cast<size_t>(parsedValue);
        return true;
    } catch (const exception&) {
        return false;
    }
}

bool tryParseMatrixSize(const string& input, size_t& matrixSize, string& errorMessage)
{
    if (input.empty()) {
        matrixSize = defaultMatrixSize;
        return true;
    }

    string cleanedInput = input;
    cleanedInput.erase(
        remove_if(cleanedInput.begin(), cleanedInput.end(), [](unsigned char character) { return isspace(character) != 0; }),
        cleanedInput.end());

    const size_t separatorPosition = cleanedInput.find_first_of("xX");
    if (separatorPosition == string::npos) {
        if (parsePositiveSize(cleanedInput, matrixSize)) {
            return true;
        }

        errorMessage = "Enter one number like 1000 or a square size like 3x3.";
        return false;
    }

    const string rowText = cleanedInput.substr(0, separatorPosition);
    const string columnText = cleanedInput.substr(separatorPosition + 1);

    size_t rows = 0;
    size_t columns = 0;

    if (!parsePositiveSize(rowText, rows) || !parsePositiveSize(columnText, columns)) {
        errorMessage = "Enter one number like 1000 or a square size like 3x3.";
        return false;
    }

    if (rows != columns) {
        errorMessage = "This program supports only N x N matrices right now. Example: 3x3.";
        return false;
    }

    matrixSize = rows;
    return true;
}

size_t promptMatrixSize()
{
    while (true) {
        cout << "Enter matrix size (example: 1000 or 3x3, press Enter for " << defaultMatrixSize << "): ";

        string input;
        getline(cin, input);

        size_t matrixSize = 0;
        string errorMessage;

        if (tryParseMatrixSize(input, matrixSize, errorMessage)) {
            return matrixSize;
        }

        cout << errorMessage << '\n';
    }
}

int promptThreadCount()
{
    const int defaultThreads = omp_get_max_threads();

    cout << "Enter thread count (press Enter for " << defaultThreads << "): ";

    string input;
    getline(cin, input);

    if (input.empty()) {
        return defaultThreads;
    }

    try {
        const int threadCount = stoi(input);
        if (threadCount > 0) {
            return threadCount;
        }
    } catch (const exception&) {
    }

    cout << "Invalid value. Using default thread count " << defaultThreads << ".\n";
    return defaultThreads;
}

vector<int> promptCompareThreadCounts()
{
    const int defaultThreads = omp_get_max_threads();

    cout << "Enter one or more thread counts for compare";
    cout << " (example: 2,4,8, press Enter for " << defaultThreads << "): ";

    string input;
    getline(cin, input);

    if (input.empty()) {
        return {defaultThreads};
    }

    for (char& character : input) {
        if (character == ',' || character == ';') {
            character = ' ';
        }
    }

    istringstream values(input);
    vector<int> threadCounts;
    string token;
    bool skippedOneThread = false;

    while (values >> token) {
        try {
            size_t processed = 0;
            const int threadCount = stoi(token, &processed);

            if (processed != token.size() || threadCount <= 0) {
                continue;
            }

            if (threadCount == 1) {
                skippedOneThread = true;
                continue;
            }

            if (find(threadCounts.begin(), threadCounts.end(), threadCount) == threadCounts.end()) {
                threadCounts.push_back(threadCount);
            }
        } catch (const exception&) {
        }
    }

    if (skippedOneThread) {
        cout << "Skipped thread count 1 because the serial run already measures one-thread work.\n";
    }

    if (!threadCounts.empty()) {
        return threadCounts;
    }

    cout << "Invalid values. Using default thread count " << defaultThreads << ".\n";
    return {defaultThreads};
}

int promptSampleCount()
{
    cout << "Enter how many times to run and average (press Enter for " << defaultRunCount << "): ";

    string input;
    getline(cin, input);

    if (input.empty()) {
        return defaultRunCount;
    }

    try {
        const int sampleCount = stoi(input);
        if (sampleCount > 0) {
            return sampleCount;
        }
    } catch (const exception&) {
    }

    cout << "Invalid value. Using default run count " << defaultRunCount << ".\n";
    return defaultRunCount;
}

bool loadMatrices(Matrix& matrixA, Matrix& matrixB)
{
    string errorMessage;
    if (!MatrixFileIO::load(matrixDataPath, matrixA, matrixB, errorMessage)) {
        cout << "Unable to load matrices: " << errorMessage << '\n';
        cout << "Generate the matrices first using menu option 1.\n";
        return false;
    }

    cout << "Loaded matrices from " << matrixDataPath << " (" << matrixA.size() << " x " << matrixA.size() << ").\n";
    return true;
}

bool saveResults(const PerformanceResult& result)
{
    string errorMessage;
    if (!appendCsv(resultsCsvPath, result, errorMessage)) {
        cout << "Unable to save results: " << errorMessage << '\n';
        return false;
    }

    return true;
}

RunExecution runSerialVersion(const Matrix& matrixA, const Matrix& matrixB, int sampleCount)
{
    Matrix result;
    const TimedSamples measuredSamples = measureAverageTime(
        [&]() { return MatrixMultiplier::multiplySerial(matrixA, matrixB); },
        result,
        sampleCount);

    RunExecution execution;
    execution.result = move(result);
    execution.resultInfo.mode = "Serial";
    execution.resultInfo.matrixSize = matrixA.size();
    execution.resultInfo.threadsUsed = 1;
    execution.resultInfo.sampleCount = sampleCount;
    execution.resultInfo.sampleTimes = measuredSamples.sampleTimes;
    execution.resultInfo.executionSeconds = measuredSamples.averageSeconds;
    execution.resultInfo.operations = MatrixMultiplier::calculateOperationCounts(matrixA.size());
    return execution;
}

RunExecution runParallelVersion(const Matrix& matrixA, const Matrix& matrixB, int threadCount, int sampleCount)
{
    Matrix result;
    const TimedSamples measuredSamples = measureAverageTime(
        [&]() { return MatrixMultiplier::multiplyParallel(matrixA, matrixB, threadCount); },
        result,
        sampleCount);

    RunExecution execution;
    execution.result = move(result);
    execution.resultInfo.mode = "Parallel";
    execution.resultInfo.matrixSize = matrixA.size();
    execution.resultInfo.threadsUsed = threadCount;
    execution.resultInfo.sampleCount = sampleCount;
    execution.resultInfo.sampleTimes = measuredSamples.sampleTimes;
    execution.resultInfo.executionSeconds = measuredSamples.averageSeconds;
    execution.resultInfo.operations = MatrixMultiplier::calculateOperationCounts(matrixA.size());
    return execution;
}

void printMenu()
{
    cout << "\n=============================================\n";
    cout << "CS421 Matrix Multiplication Program\n";
    cout << "Shows average time, speedup, and efficiency\n";
    cout << "=============================================\n";
    cout << "1. Generate random matrices\n";
    cout << "2. Run serial multiplication\n";
    cout << "3. Run parallel multiplication\n";
    cout << "4. Compare serial and parallel\n";
    cout << "5. Exit\n";
    cout << "Choose an option: ";
}

void handleGenerate()
{
    const size_t matrixSize = promptMatrixSize();

    CreateMatrixData generator;
    auto [matrixA, matrixB] = generator.generatePair(matrixSize);

    string errorMessage;
    if (!MatrixFileIO::save(matrixDataPath, matrixA, matrixB, errorMessage)) {
        cout << "Failed to save matrices: " << errorMessage << '\n';
        return;
    }

    cout << "Generated two random " << matrixSize << " x " << matrixSize
         << " matrices and saved them to " << matrixDataPath << ".\n";
}

void handleSerial()
{
    Matrix matrixA;
    Matrix matrixB;

    if (!loadMatrices(matrixA, matrixB)) {
        return;
    }

    const int sampleCount = promptSampleCount();
    RunExecution execution = runSerialVersion(matrixA, matrixB, sampleCount);
    printResult(execution.resultInfo);

    if (saveResults(execution.resultInfo)) {
        cout << "Results saved to " << resultsCsvPath << ".\n";
    }
}

void handleParallel()
{
    Matrix matrixA;
    Matrix matrixB;

    if (!loadMatrices(matrixA, matrixB)) {
        return;
    }

    const int sampleCount = promptSampleCount();
    const int threadCount = promptThreadCount();
    RunExecution execution = runParallelVersion(matrixA, matrixB, threadCount, sampleCount);
    printResult(execution.resultInfo);

    if (saveResults(execution.resultInfo)) {
        cout << "Results saved to " << resultsCsvPath << ".\n";
    }
}

void handleCompare()
{
    Matrix matrixA;
    Matrix matrixB;

    if (!loadMatrices(matrixA, matrixB)) {
        return;
    }

    const int sampleCount = promptSampleCount();
    const vector<int> threadCounts = promptCompareThreadCounts();

    RunExecution serialExecution = runSerialVersion(matrixA, matrixB, sampleCount);

    serialExecution.resultInfo.hasComparisonData = true;
    serialExecution.resultInfo.speedup = 1.0;
    serialExecution.resultInfo.efficiency = 1.0;
    serialExecution.resultInfo.hasVerification = true;
    serialExecution.resultInfo.verified = true;

    vector<PerformanceResult> parallelResults;
    parallelResults.reserve(threadCounts.size());
    bool allVerified = true;

    for (int threadCount : threadCounts) {
        RunExecution parallelExecution = runParallelVersion(matrixA, matrixB, threadCount, sampleCount);

        // Use the serial result to check if the parallel result is correct.
        const bool verified = MatrixMultiplier::areEqual(
            serialExecution.result,
            parallelExecution.result,
            comparisonTolerance);

        parallelExecution.resultInfo.hasComparisonData = true;
        parallelExecution.resultInfo.hasVerification = true;
        if (parallelExecution.resultInfo.executionSeconds > 0.0) {
            parallelExecution.resultInfo.speedup =
                serialExecution.resultInfo.executionSeconds / parallelExecution.resultInfo.executionSeconds;
            parallelExecution.resultInfo.efficiency =
            parallelExecution.resultInfo.speedup / static_cast<double>(parallelExecution.resultInfo.threadsUsed);
        }
        parallelExecution.resultInfo.verified = verified;
        allVerified = allVerified && verified;
        parallelResults.push_back(parallelExecution.resultInfo);
    }

    serialExecution.resultInfo.verified = allVerified;
    printResult(serialExecution.resultInfo);

    bool savedAllResults = saveResults(serialExecution.resultInfo);

    for (const PerformanceResult& parallelResult : parallelResults) {
        printResult(parallelResult);
        printComparison(serialExecution.resultInfo, parallelResult);

        if (!saveResults(parallelResult)) {
            savedAllResults = false;
        }
    }

    if (savedAllResults) {
        cout << "Comparison results saved to " << resultsCsvPath << ".\n";
    }
}

}  // namespace

int ProgramController::run()
{
    while (true) {
        printMenu();

        string input;
        getline(cin, input);

        if (input == "1") {
            handleGenerate();
        } else if (input == "2") {
            handleSerial();
        } else if (input == "3") {
            handleParallel();
        } else if (input == "4") {
            handleCompare();
        } else if (input == "5") {
            cout << "Exiting matrix multiplication program.\n";
            return 0;
        } else {
            cout << "Invalid option. Please choose 1, 2, 3, 4, or 5.\n";
        }

        cout << '\n';
    }
}
