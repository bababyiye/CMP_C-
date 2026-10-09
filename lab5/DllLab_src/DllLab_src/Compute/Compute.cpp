// Compute.cpp
// Purpose: implementation of Compute.dll - a brute-force (full
//          enumeration) primality counter that splits its input across
//          a configurable number of worker threads, with an artificial
//          memory load and a hard cap on the thread count (task 1-2 of
//          the lab).
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "Compute.h"
#include <thread>
#include <algorithm>

namespace
{
    // Trial-division primality test: checks every odd divisor up to
    // sqrt(n) one by one - the "full enumeration" algorithm the lab asks
    // for, applied to a single candidate number.
    bool IsPrime(int n)
    {
        if (n < 2)
            return false;
        if (n % 2 == 0)
            return n == 2;

        for (int d = 3; static_cast<long long>(d) * d <= n; d += 2)
        {
            if (n % d == 0)
                return false;
        }
        return true;
    }
}

void Compute::SetData(const std::vector<int>& data)
{
    m_data = data;
    m_result = 0;
}

long long Compute::CountPrimesInRange(const std::vector<int>& data, size_t begin, size_t end)
{
    long long count = 0;
    for (size_t i = begin; i < end; ++i)
    {
        if (IsPrime(data[i]))
            ++count;
    }
    return count;
}

void Compute::Run(int threadsCount)
{
    // Clamp the requested thread count to a sane range (task "можливість
    // обмежувати кількість потоків"): never fewer than 1, and never more
    // than a small multiple of the hardware's own thread count, so a
    // mistaken or malicious argument cannot spawn an unreasonable number
    // of OS threads.
    unsigned hwThreads = std::thread::hardware_concurrency();
    if (hwThreads == 0)
        hwThreads = 4;
    int maxThreads = static_cast<int>(hwThreads) * 4;

    if (threadsCount < 1)
        threadsCount = 1;
    if (threadsCount > maxThreads)
        threadsCount = maxThreads;

    // Artificial memory load: allocate a scratch buffer proportional to
    // the input size and touch one byte per 4 KB page so every page is
    // actually committed by the OS, not just reserved. The buffer is kept
    // in m_memoryLoadBuffer (a member, not a local variable) so it stays
    // resident until the next SetData()/Release() call, which is what
    // lets MainApp.exe's resource monitor observe the working-set bump.
    m_memoryLoadBuffer.assign(m_data.size() * 64, 0);
    for (size_t i = 0; i < m_memoryLoadBuffer.size(); i += 4096)
        m_memoryLoadBuffer[i] = static_cast<char>(i);

    if (m_data.empty())
    {
        m_result = 0;
        return;
    }

    size_t total = m_data.size();
    size_t chunk = (total + static_cast<size_t>(threadsCount) - 1) / static_cast<size_t>(threadsCount);

    std::vector<std::thread> workers;
    std::vector<long long> partialResults(threadsCount, 0);

    for (int t = 0; t < threadsCount; ++t)
    {
        size_t begin = static_cast<size_t>(t) * chunk;
        size_t end = std::min(total, begin + chunk);
        if (begin >= end)
            continue;

        workers.emplace_back([this, &partialResults, t, begin, end]()
        {
            partialResults[static_cast<size_t>(t)] = CountPrimesInRange(m_data, begin, end);
        });
    }

    for (auto& worker : workers)
        worker.join();

    long long total_sum = 0;
    for (long long partial : partialResults)
        total_sum += partial;

    m_result = total_sum;
}

long long Compute::GetResult()
{
    return m_result;
}

const char* Compute::GetVersion()
{
    return "Compute.dll v1.0 (brute-force primality counter)";
}

void Compute::Release()
{
    delete this;
}

extern "C" __declspec(dllexport) ICompute* CreateCompute()
{
    return new Compute();
}

// end of file
