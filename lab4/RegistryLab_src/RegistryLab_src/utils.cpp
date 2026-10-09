// utils.cpp
// Purpose: implementation of small helper functions shared by all
//          modules of the Registry lab utility.
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "utils.h"
#include <iostream>
#include <sstream>
#include <iomanip>

void PrintSectionTitle(const std::string& title)
{
    std::cout << "\n============================================================\n";
    std::cout << title << "\n";
    std::cout << "============================================================\n";
}

std::string RegistryTypeToString(DWORD type)
{
    switch (type)
    {
        case REG_SZ:        return "REG_SZ";
        case REG_EXPAND_SZ: return "REG_EXPAND_SZ";
        case REG_BINARY:    return "REG_BINARY";
        case REG_DWORD:     return "REG_DWORD";
        case REG_QWORD:     return "REG_QWORD";
        case REG_MULTI_SZ:  return "REG_MULTI_SZ";
        case REG_NONE:      return "REG_NONE";
        default:            return "UNKNOWN";
    }
}

std::string BytesToHex(const BYTE* data, DWORD size)
{
    std::ostringstream oss;
    for (DWORD i = 0; i < size; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]);
        if (i + 1 < size)
            oss << ",";
    }
    return oss.str();
}

std::string EscapeForRegFile(const std::string& input)
{
    std::string result;
    result.reserve(input.size());
    for (char c : input)
    {
        if (c == '\\' || c == '"')
            result.push_back('\\');
        result.push_back(c);
    }
    return result;
}

// end of file
