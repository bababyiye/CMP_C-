// TaskScheduler.cpp
// Purpose: implementation of Task Scheduler enumeration via the
//          registry (task 7 of the lab).
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "TaskScheduler.h"
#include "utils.h"
#include <windows.h>
#include <iostream>

namespace
{
    const char* kTasksKeyPath =
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Schedule\\TaskCache\\Tasks";

    // Reads one named value of a task subkey, if present, and prints a
    // short summary line for it (used for the "Path" string value and
    // for the "Actions" / "Triggers" binary blobs).
    void PrintValueIfPresent(HKEY hTaskKey, const char* valueName, const char* label)
    {
        BYTE buffer[8192];
        DWORD size = sizeof(buffer);
        DWORD type = 0;

        LONG result = RegQueryValueExA(
            hTaskKey, valueName, nullptr, &type, buffer, &size);

        if (result != ERROR_SUCCESS)
        {
            std::cout << "        " << label << ": (not present)\n";
            return;
        }

        if (type == REG_SZ || type == REG_EXPAND_SZ)
        {
            std::string value(reinterpret_cast<char*>(buffer), size);
            while (!value.empty() && value.back() == '\0')
                value.pop_back();
            std::cout << "        " << label << ": " << value << "\n";
        }
        else
        {
            // Actions/Triggers are stored as an undocumented binary
            // structure; only its size is reported here.
            std::cout << "        " << label << ": binary data, "
                      << size << " bytes (" << RegistryTypeToString(type) << ")\n";
        }
    }
}

void ListScheduledTasksFromRegistry()
{
    PrintSectionTitle("SCHEDULED TASKS FROM REGISTRY (task 7)");
    std::cout << "Source: HKCU\\" << kTasksKeyPath << "\n";
    std::cout << "(current user's hive, as explicitly allowed by the lab description)\n";

    HKEY hTasksKey;
    LONG openResult = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE, kTasksKeyPath, 0, KEY_READ, &hTasksKey);
    if (openResult != ERROR_SUCCESS)
    {
        std::cout << "Failed to open the Tasks key. Error code: " << openResult << "\n";
        return;
    }

    char taskId[256];
    for (DWORD index = 0; ; ++index)
    {
        DWORD idSize = sizeof(taskId);
        LONG enumResult = RegEnumKeyExA(
            hTasksKey, index, taskId, &idSize,
            nullptr, nullptr, nullptr, nullptr);

        if (enumResult == ERROR_NO_MORE_ITEMS)
            break;

        if (enumResult != ERROR_SUCCESS)
        {
            std::cout << "RegEnumKeyEx failed at index " << index
                      << ", error code: " << enumResult << "\n";
            break;
        }

        HKEY hTaskKey;
        LONG openTaskResult = RegOpenKeyExA(hTasksKey, taskId, 0, KEY_READ, &hTaskKey);
        if (openTaskResult != ERROR_SUCCESS)
        {
            std::cout << "\nTask ID: " << taskId
                      << " -> failed to open, error code: " << openTaskResult << "\n";
            continue;
        }

        std::cout << "\nTask ID: " << taskId << "\n";
        PrintValueIfPresent(hTaskKey, "Path", "Task path (name)");
        PrintValueIfPresent(hTaskKey, "Actions", "Actions");
        PrintValueIfPresent(hTaskKey, "Triggers", "Triggers");

        RegCloseKey(hTaskKey);
    }

    RegCloseKey(hTasksKey);
}

// end of file
