// main.cpp
// Purpose: entry point of MainApp.exe. Statically imports Interface.dll
//          (task 2.1), dynamically loads/unloads Compute.dll (task 2.2),
//          runs a benchmark matrix across different thread counts and
//          data sizes while logging resource usage (task 4), and ranks
//          the results using a ready-made library function (task 3).
// Author:  Zinchenko Maksym, group IPZ-23-5
#include <windows.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <string>

#include "Interface.h"
#include "ResourceMonitor.h"
#include "ThirdPartyDemo.h"

namespace
{
    // Generates a reproducible array of random candidate numbers for the
    // primality benchmark. A fixed seed is used so that repeated runs of
    // the program are comparable to one another.
    std::vector<int> GenerateRandomData(size_t size)
    {
        std::vector<int> data(size);
        std::mt19937 rng(12345);
        std::uniform_int_distribution<int> dist(2, 50000);
        for (size_t i = 0; i < size; ++i)
            data[i] = dist(rng);
        return data;
    }
}

int main()
{
    std::cout << "=====================================================\n";
    std::cout << "   DLL demo: Interface.dll + Compute.dll + MainApp    \n";
    std::cout << "=====================================================\n";

    // ---- Task 2.1: static linking of Interface.dll ----
    // Interface.dll is loaded automatically by the Windows loader before
    // main() even runs, because MainApp.exe is linked against
    // Interface.lib (the import library produced when Interface.dll was
    // built). Calling GetInterfaceVersion() here only proves the link
    // succeeded; the DLL itself was already mapped into memory earlier.
    std::cout << "\n[Interface.dll] " << GetInterfaceVersion() << "\n";

    // ---- Task 2.2: dynamic loading of Compute.dll ----
    std::cout << "\nChecking whether Compute.dll is already loaded...\n";
    HMODULE existing = GetModuleHandleW(L"Compute.dll");
    std::cout << (existing ? "Compute.dll is already in memory.\n"
                            : "Compute.dll is not loaded yet.\n");

    std::cout << "Loading Compute.dll with LoadLibraryW...\n";
    HMODULE hCompute = LoadLibraryW(L"Compute.dll");
    if (!hCompute)
    {
        std::cout << "Failed to load Compute.dll. Error code: " << GetLastError() << "\n";
        return 1;
    }

    // GetProcAddress always returns the generic FARPROC function-pointer
    // type, so casting it to our specific CreateCompute_t signature is
    // the standard, unavoidable WinAPI pattern here; the compiler cannot
    // check the real signature behind the exported name, so this warning
    // is suppressed locally rather than for the whole file.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    auto createCompute = reinterpret_cast<CreateCompute_t>(
        GetProcAddress(hCompute, "CreateCompute"));
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    if (!createCompute)
    {
        std::cout << "Failed to find CreateCompute in Compute.dll.\n";
        FreeLibrary(hCompute);
        return 1;
    }

    ICompute* compute = createCompute();
    std::cout << "[Compute.dll]   " << compute->GetVersion() << "\n";

    // ---- Task 4.2: vary thread count and data size ----
    // ---- Task 4.1: log resource usage for every combination ----
    const std::vector<size_t> dataSizes = { 1000000, 4000000 };
    const std::vector<int> threadCounts = { 1, 2, 4, 8 };
    const std::string logFile = "resource_log.csv";

    std::vector<BenchmarkResult> results;

    std::cout << "\nRunning benchmark matrix (" << dataSizes.size() << " data sizes x "
              << threadCounts.size() << " thread counts)...\n";

    for (size_t dataSize : dataSizes)
    {
        std::vector<int> data = GenerateRandomData(dataSize);

        for (int threads : threadCounts)
        {
            compute->SetData(data);

            std::string phaseLabel = "size=" + std::to_string(dataSize) +
                                      ";threads=" + std::to_string(threads);

            ResourceMonitor monitor;
            monitor.Start(logFile, phaseLabel, 50);

            auto start = std::chrono::steady_clock::now();
            compute->Run(threads);
            auto finish = std::chrono::steady_clock::now();

            monitor.Stop();

            double elapsedMs = std::chrono::duration<double, std::milli>(finish - start).count();
            long long primes = compute->GetResult();

            std::cout << "  size=" << dataSize << " threads=" << threads
                      << " time=" << elapsedMs << " ms primes=" << primes << "\n";

            results.push_back(BenchmarkResult{ dataSize, threads, elapsedMs, primes });
        }
    }

    // ---- Task 3: use of a ready-made library (<algorithm> std::sort) ----
    SortResultsByElapsedTime(results);
    PrintRankedResults(results);

    // ---- Unload Compute.dll ----
    compute->Release();
    FreeLibrary(hCompute);

    HMODULE afterUnload = GetModuleHandleW(L"Compute.dll");
    std::cout << "\nCompute.dll after FreeLibrary: "
              << (afterUnload ? "still loaded\n" : "unloaded\n");

    std::cout << "\nResource log written to \"" << logFile << "\".\n";
    std::cout << "Open it in Excel / Google Sheets / Python and build charts of\n";
    std::cout << "working_set_mb and thread_count over elapsed_ms for each phase\n";
    std::cout << "(task 4.1).\n";

    return 0;
}

// end of file
