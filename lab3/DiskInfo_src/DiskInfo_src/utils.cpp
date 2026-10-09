// utils.cpp
// Purpose: implementation of small helper functions shared by all modules
//          of the DiskInfo utility.
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "utils.h"
#include <sstream>
#include <iomanip>
#include <iostream>

std::string FormatBytes(unsigned long long bytes)
{
    const char* units[] = { "B", "KB", "MB", "GB", "TB", "PB" };
    double value = static_cast<double>(bytes);
    int unitIndex = 0;

    while (value >= 1024.0 && unitIndex < 5)
    {
        value /= 1024.0;
        ++unitIndex;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value << " " << units[unitIndex];
    return oss.str();
}

std::string FormatPercent(unsigned long long part, unsigned long long whole)
{
    std::ostringstream oss;
    if (whole == 0)
    {
        oss << "0.0%";
        return oss.str();
    }
    double percent = (static_cast<double>(part) / static_cast<double>(whole)) * 100.0;
    oss << std::fixed << std::setprecision(1) << percent << "%";
    return oss.str();
}

std::string GetTimestamp()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    char buffer[32];
    sprintf_s(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    return std::string(buffer);
}

void PrintSectionTitle(const std::string& title)
{
    std::cout << "\n============================================================\n";
    std::cout << title << "\n";
    std::cout << "============================================================\n";
}

// end of file
