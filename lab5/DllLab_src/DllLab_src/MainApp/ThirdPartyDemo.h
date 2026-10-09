// ThirdPartyDemo.h
// Purpose: declares a small module whose job is only to exercise a
//          ready-made library function, std::sort from <algorithm>
//          (task 3 - "Модуль підключення сторонньої бібліотеки"). The
//          ranking logic itself is entirely std::sort's own
//          implementation; this module only supplies the comparison
//          rule and prints the result.
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

#include <vector>
#include <cstddef>

struct BenchmarkResult
{
    size_t dataSize;
    int threadsCount;
    double elapsedMs;
    long long primesFound;
};

// Sorts results in place, fastest run first, using std::sort.
void SortResultsByElapsedTime(std::vector<BenchmarkResult>& results);

// Prints a short ranked summary table to stdout.
void PrintRankedResults(const std::vector<BenchmarkResult>& results);

// end of file
