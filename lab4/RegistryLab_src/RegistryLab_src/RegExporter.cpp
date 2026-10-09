// RegExporter.cpp
// Purpose: implementation of the recursive .reg file exporter
//          (task 8, variant 1 of the lab).
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "RegExporter.h"
#include "utils.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

namespace
{
    // Converts an ANSI byte string (as returned by the "A" registry
    // functions) into UTF-16LE and then into the comma-separated
    // hexadecimal byte form RegEdit uses for hex(2)/hex(7) values.
    std::string AnsiBytesToUtf16Hex(const char* data, DWORD sizeBytes)
    {
        int wideLen = MultiByteToWideChar(CP_ACP, 0, data, static_cast<int>(sizeBytes), nullptr, 0);
        std::vector<wchar_t> wide(wideLen);
        MultiByteToWideChar(CP_ACP, 0, data, static_cast<int>(sizeBytes), wide.data(), wideLen);

        std::ostringstream oss;
        const BYTE* bytePtr = reinterpret_cast<const BYTE*>(wide.data());
        DWORD byteCount = static_cast<DWORD>(wideLen) * sizeof(wchar_t);
        for (DWORD i = 0; i < byteCount; ++i)
        {
            oss << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(bytePtr[i]);
            if (i + 1 < byteCount)
                oss << ",";
        }
        return oss.str();
    }

    // Formats one value's data according to its registry type, in the
    // exact textual form RegEdit itself writes into a .reg file.
    std::string FormatRegValue(DWORD type, const BYTE* data, DWORD size)
    {
        std::ostringstream oss;
        switch (type)
        {
            case REG_SZ:
            {
                std::string text(reinterpret_cast<const char*>(data), size);
                while (!text.empty() && text.back() == '\0')
                    text.pop_back();
                oss << "\"" << EscapeForRegFile(text) << "\"";
                break;
            }
            case REG_EXPAND_SZ:
            {
                oss << "hex(2):"
                    << AnsiBytesToUtf16Hex(reinterpret_cast<const char*>(data), size);
                break;
            }
            case REG_MULTI_SZ:
            {
                oss << "hex(7):"
                    << AnsiBytesToUtf16Hex(reinterpret_cast<const char*>(data), size);
                break;
            }
            case REG_DWORD:
            {
                DWORD value = 0;
                if (size >= sizeof(DWORD))
                    value = *reinterpret_cast<const DWORD*>(data);
                oss << "dword:" << std::hex << std::setw(8) << std::setfill('0') << value;
                break;
            }
            case REG_QWORD:
            {
                oss << "hex(b):" << BytesToHex(data, size);
                break;
            }
            case REG_BINARY:
            {
                oss << "hex:" << BytesToHex(data, size);
                break;
            }
            default:
            {
                oss << "hex(" << type << "):" << BytesToHex(data, size);
                break;
            }
        }
        return oss.str();
    }

    // Writes the [path] header, every value directly under hKey, and
    // then recurses into every subkey, appending everything to out.
    void ExportKeyRecursive(HKEY hKey, const std::string& fullPath, std::ofstream& out)
    {
        out << "[" << fullPath << "]\n";

        char valueName[256];
        BYTE valueData[16384];

        for (DWORD index = 0; ; ++index)
        {
            DWORD nameSize = sizeof(valueName);
            DWORD dataSize = sizeof(valueData);
            DWORD type = 0;

            LONG result = RegEnumValueA(
                hKey, index, valueName, &nameSize,
                nullptr, &type, valueData, &dataSize);

            if (result == ERROR_NO_MORE_ITEMS)
                break;
            if (result != ERROR_SUCCESS)
                break;

            std::string nameField = (nameSize == 0)
                ? "@"
                : ("\"" + EscapeForRegFile(valueName) + "\"");

            out << nameField << "=" << FormatRegValue(type, valueData, dataSize) << "\n";
        }

        out << "\n";

        char subKeyName[256];
        for (DWORD index = 0; ; ++index)
        {
            DWORD nameSize = sizeof(subKeyName);
            LONG result = RegEnumKeyExA(
                hKey, index, subKeyName, &nameSize,
                nullptr, nullptr, nullptr, nullptr);

            if (result == ERROR_NO_MORE_ITEMS)
                break;
            if (result != ERROR_SUCCESS)
                break;

            HKEY hSubKey;
            if (RegOpenKeyExA(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
            {
                ExportKeyRecursive(hSubKey, fullPath + "\\" + subKeyName, out);
                RegCloseKey(hSubKey);
            }
        }
    }
}

bool ExportKeyToRegFile(HKEY root,
                         const std::string& rootName,
                         const std::string& subKey,
                         const std::string& outputFile)
{
    PrintSectionTitle("EXPORT REGISTRY KEY TO .reg FILE (task 8)");

    HKEY hKey;
    LONG openResult = RegOpenKeyExA(root, subKey.c_str(), 0, KEY_READ, &hKey);
    if (openResult != ERROR_SUCCESS)
    {
        std::cout << "Failed to open " << rootName << "\\" << subKey
                  << ". Error code: " << openResult << "\n";
        return false;
    }

    std::ofstream out(outputFile, std::ios::trunc);
    if (!out.is_open())
    {
        std::cout << "Failed to create output file \"" << outputFile << "\".\n";
        RegCloseKey(hKey);
        return false;
    }

    out << "Windows Registry Editor Version 5.00\n\n";
    ExportKeyRecursive(hKey, rootName + "\\" + subKey, out);

    out.close();
    RegCloseKey(hKey);

    std::cout << "Key " << rootName << "\\" << subKey
              << " was exported to \"" << outputFile << "\".\n";
    return true;
}

// end of file
