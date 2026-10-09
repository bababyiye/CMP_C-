// ThirdPartyDemo.cpp
// Purpose: implementation of the <algorithm> demonstration module
//          (task 3 of the lab).
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "ThirdPartyDemo.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

void SortResultsByElapsedTime(std::vector<BenchmarkResult>& results)
{
    std::sort(results.begin(), results.end(),
        [](const BenchmarkResult& a, const BenchmarkResult& b)
        {
            return a.elapsedMs < b.elapsedMs;
        });
}

void PrintRankedResults(const std::vector<BenchmarkResult>& results)
{
    std::cout << "\nRanking (fastest -> slowest), via std::sort from <algorithm>:\n";
    for (size_t i = 0; i < results.size(); ++i)
    {
        const auto& r = results[i];
        std::cout << "  " << (i + 1) << ". size=" << r.dataSize
                  << " threads=" << r.threadsCount
                  << " time=" << std::fixed << std::setprecision(1) << r.elapsedMs << " ms"
                  << " primes=" << r.primesFound << "\n";
    }
}

// end of file
