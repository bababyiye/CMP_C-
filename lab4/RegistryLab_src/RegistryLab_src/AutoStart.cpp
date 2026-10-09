// AutoStart.cpp
// Purpose: implementation of autostart registry enumeration and
//          modification (tasks 4-6 of the lab).
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "AutoStart.h"
#include "utils.h"
#include <windows.h>
#include <iostream>
#include <vector>

namespace
{
    // One autostart location to scan: which hive, its display name and
    // the sub-key path inside that hive.
    struct AutoStartLocation
    {
        HKEY root;
        const char* rootName;
        const char* subKey;
    };

    const AutoStartLocation kLocations[] = {
        { HKEY_LOCAL_MACHINE, "HKLM", "Software\\Microsoft\\Windows\\CurrentVersion\\Run" },
        { HKEY_LOCAL_MACHINE, "HKLM", "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce" },
        { HKEY_CURRENT_USER,  "HKCU", "Software\\Microsoft\\Windows\\CurrentVersion\\Run" },
        { HKEY_CURRENT_USER,  "HKCU", "Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce" },
    };

    // Prints every value stored directly under an already-open key.
    void PrintValuesOfKey(HKEY hKey, const char* rootName, const char* subKey)
    {
        char valueName[256];
        BYTE valueData[16384];

        for (DWORD index = 0; ; ++index)
        {
            DWORD nameSize = sizeof(valueName);
            DWORD dataSize = sizeof(valueData);
            DWORD type = 0;

            LONG result = RegEnumValueA(
                hKey, index,
                valueName, &nameSize,
                nullptr,
                &type,
                valueData, &dataSize);

            if (result == ERROR_NO_MORE_ITEMS)
                break;

            if (result != ERROR_SUCCESS)
            {
                std::cout << "    RegEnumValue failed at index " << index
                          << ", error code: " << result << "\n";
                break;
            }

            std::string displayValue;
            if (type == REG_SZ || type == REG_EXPAND_SZ)
            {
                displayValue.assign(reinterpret_cast<char*>(valueData), dataSize);
                // Trim a possible trailing null terminator copied with the data.
                while (!displayValue.empty() && displayValue.back() == '\0')
                    displayValue.pop_back();
            }
            else
            {
                displayValue = "(" + RegistryTypeToString(type) + ") " +
                    BytesToHex(valueData, dataSize);
            }

            std::cout << "    [" << rootName << "] " << subKey << "\n";
            std::cout << "        Name:   " << (valueName[0] ? valueName : "(default)") << "\n";
            std::cout << "        Value:  " << displayValue << "\n";
            std::cout << "        Source: " << rootName << "\n";
        }
    }
}

void ListAutoStartEntries()
{
    PrintSectionTitle("AUTOSTART ENTRIES (task 4 / 6)");

    for (const auto& location : kLocations)
    {
        HKEY hKey;
        LONG openResult = RegOpenKeyExA(
            location.root, location.subKey, 0, KEY_READ, &hKey);

        std::cout << "\n--- " << location.rootName << "\\" << location.subKey << " ---\n";

        if (openResult != ERROR_SUCCESS)
        {
            std::cout << "    Failed to open key. Error code: " << openResult << "\n";
            continue;
        }

        PrintValuesOfKey(hKey, location.rootName, location.subKey);
        RegCloseKey(hKey);
    }
}

bool AddAutoStartEntry(const std::string& valueName, const std::string& applicationPath)
{
    PrintSectionTitle("ADD AUTOSTART ENTRY (task 5)");

    HKEY hKey;
    LONG openResult = RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey);

    if (openResult != ERROR_SUCCESS)
    {
        std::cout << "Failed to open HKCU...\\Run for writing. Error code: "
                  << openResult << "\n";
        return false;
    }

    // +1 to also store the terminating null byte, as REG_SZ requires.
    LONG setResult = RegSetValueExA(
        hKey,
        valueName.c_str(),
        0,
        REG_SZ,
        reinterpret_cast<const BYTE*>(applicationPath.c_str()),
        static_cast<DWORD>(applicationPath.size() + 1));

    RegCloseKey(hKey);

    if (setResult != ERROR_SUCCESS)
    {
        std::cout << "Failed to write value \"" << valueName
                  << "\". Error code: " << setResult << "\n";
        return false;
    }

    std::cout << "Value \"" << valueName << "\" = \"" << applicationPath
              << "\" was written to HKCU\\...\\Run.\n";
    return true;
}

// end of file
