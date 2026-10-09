// MemoryInfo.cpp
// Purpose: implementation of system memory information retrieval
//          (task 5 of the lab: GlobalMemoryStatusEx).
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "MemoryInfo.h"
#include "utils.h"
#include <windows.h>
#include <iostream>

void PrintMemoryInfo()
{
    PrintSectionTitle("SYSTEM MEMORY INFORMATION");

    // MEMORYSTATUSEX must have its dwLength member set before the call,
    // otherwise GlobalMemoryStatusEx fails with ERROR_INVALID_PARAMETER.
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);

    if (!GlobalMemoryStatusEx(&status))
    {
        std::cout << "Failed to obtain memory status. Error code: "
                  << GetLastError() << "\n";
        return;
    }

    std::cout << "RAM load:              " << status.dwMemoryLoad << "%\n";
    std::cout << "Total physical memory:  " << FormatBytes(status.ullTotalPhys) << "\n";
    std::cout << "Free physical memory:   " << FormatBytes(status.ullAvailPhys) << "\n";
    std::cout << "Total page file:        " << FormatBytes(status.ullTotalPageFile) << "\n";
    std::cout << "Free page file:         " << FormatBytes(status.ullAvailPageFile) << "\n";
    std::cout << "Total virtual memory:   " << FormatBytes(status.ullTotalVirtual) << "\n";
    std::cout << "Free virtual memory:    " << FormatBytes(status.ullAvailVirtual) << "\n";
}

// end of file
