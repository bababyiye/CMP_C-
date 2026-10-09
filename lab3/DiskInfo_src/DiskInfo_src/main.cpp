// main.cpp
// Purpose: entry point of the DiskInfo utility. Calls, in order, every
//          information module required by the lab (disks, memory, user,
//          system directories) and then starts directory-change
//          monitoring for a directory chosen by the user.
// Author:  Zinchenko Maksym, group IPZ-23-5
#include <windows.h>
#include <iostream>
#include <string>

#include "DiskInfo.h"
#include "MemoryInfo.h"
#include "UserInfo.h"
#include "Watcher.h"

int main()
{
    std::cout << "=====================================================\n";
    std::cout << "           DiskInfo - system information utility     \n";
    std::cout << "=====================================================\n";

    // ---- Tasks 1-4: logical drives, their type, file system, space ----
    std::vector<std::string> drives = GetLogicalDriveList();

    PrintLogicalDrives(drives);
    PrintDriveTypes(drives);
    PrintVolumeInformation(drives);
    PrintDiskSpace(drives);

    // ---- Task 5: system memory ----
    PrintMemoryInfo();

    // ---- Task 6-7: computer name and current user ----
    PrintComputerName();
    PrintUserName();

    // ---- Task 8: system directories ----
    PrintSystemDirectories();

    // ---- Task 9: directory change monitoring ----
    std::cout << "\nEnter a directory path to monitor for changes\n"
              << "(press ENTER to use the current directory): ";
    std::string path;
    std::getline(std::cin, path);
    if (path.empty())
    {
        char currentDir[MAX_PATH] = { 0 };
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        path = currentDir;
    }

    // Watch for three kinds of events, as required by the lab: file name
    // changes (create/delete/rename), file modification (write) and
    // attribute changes.
    DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME
                 | FILE_NOTIFY_CHANGE_LAST_WRITE
                 | FILE_NOTIFY_CHANGE_ATTRIBUTES;

    WatchDirectory(path, "diskinfo_watch.log", filter, false);

    std::cout << "\nDone. See diskinfo_watch.log for the full event log.\n";
    return 0;
}

// end of file
