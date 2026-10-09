// ResourceMonitor.cpp
// Purpose: implementation of the background resource sampler (task 4.1).
//          Uses Psapi.h (GetProcessMemoryInfo) and the Tool Help library
//          (CreateToolhelp32Snapshot) from the Windows SDK - the
//          "ready-made system library" required to be used somewhere in
//          the project alongside the student's own code.
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "ResourceMonitor.h"
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <fstream>
#include <chrono>

// Lets Visual Studio link Psapi.lib automatically, without having to add
// it by hand under Project Properties -> Linker -> Input. Ignored by
// MinGW, which is instead linked with the -lpsapi command-line flag.
#if defined(_MSC_VER)
#pragma comment(lib, "psapi.lib")
#endif

namespace
{
    // Counts how many OS threads currently belong to this process, by
    // walking a system-wide snapshot of all threads and keeping only the
    // ones whose owner process ID matches ours.
    int CountOwnThreads()
    {
        DWORD pid = GetCurrentProcessId();
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
            return -1;

        int count = 0;
        THREADENTRY32 entry;
        entry.dwSize = sizeof(entry);

        if (Thread32First(snapshot, &entry))
        {
            do
            {
                if (entry.th32OwnerProcessID == pid)
                    ++count;
            } while (Thread32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return count;
    }

    // Returns the process's current working-set size in megabytes.
    double GetWorkingSetMB()
    {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
            return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
        return -1.0;
    }
}

void ResourceMonitor::Start(const std::string& logFilePath, const std::string& phaseLabel, int intervalMs)
{
    m_logFilePath = logFilePath;
    m_phaseLabel = phaseLabel;
    m_intervalMs = intervalMs;
    m_running = true;
    m_thread = std::thread(&ResourceMonitor::SamplingLoop, this);
}

void ResourceMonitor::Stop()
{
    if (m_running)
    {
        m_running = false;
        if (m_thread.joinable())
            m_thread.join();
    }
}

ResourceMonitor::~ResourceMonitor()
{
    Stop();
}

void ResourceMonitor::SamplingLoop()
{
    bool fileExisted = false;
    {
        std::ifstream testFile(m_logFilePath);
        fileExisted = testFile.good();
    }

    std::ofstream out(m_logFilePath, std::ios::app);
    if (!out.is_open())
        return;

    if (!fileExisted)
        out << "elapsed_ms,phase,working_set_mb,thread_count\n";

    auto start = std::chrono::steady_clock::now();

    while (m_running)
    {
        auto now = std::chrono::steady_clock::now();
        long long elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

        out << elapsedMs << "," << m_phaseLabel << ","
            << GetWorkingSetMB() << "," << CountOwnThreads() << "\n";
        out.flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMs));
    }
}

// end of file
