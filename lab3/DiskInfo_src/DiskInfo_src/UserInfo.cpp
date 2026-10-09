// UserInfo.cpp
// Purpose: implementation of computer name / user name / system
//          directories retrieval (tasks 6, 7, 8 of the lab).
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "UserInfo.h"
#include "utils.h"
#include <windows.h>
#include <iostream>
#include <vector>

void PrintComputerName()
{
    PrintSectionTitle("COMPUTER NAME");

    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buffer);

    if (GetComputerNameA(buffer, &size))
    {
        std::cout << "NetBIOS computer name: " << buffer << "\n";
    }
    else
    {
        std::cout << "Failed to obtain computer name. Error code: "
                  << GetLastError() << "\n";
    }
}

void PrintUserName()
{
    PrintSectionTitle("CURRENT USER");

    char buffer[256];
    DWORD size = sizeof(buffer);

    if (GetUserNameA(buffer, &size))
    {
        std::cout << "Current user name: " << buffer << "\n";
    }
    else
    {
        std::cout << "Failed to obtain user name. Error code: "
                  << GetLastError() << "\n";
    }
}

void PrintSystemDirectories()
{
    PrintSectionTitle("SYSTEM DIRECTORIES");

    char sysDir[MAX_PATH]   = { 0 };
    char winDir[MAX_PATH]   = { 0 };
    char tempDir[MAX_PATH]  = { 0 };
    char currDir[MAX_PATH]  = { 0 };

    if (GetSystemDirectoryA(sysDir, MAX_PATH) == 0)
        std::cout << "Failed to obtain system directory.\n";
    else
        std::cout << "System directory:   " << sysDir << "\n";

    if (GetWindowsDirectoryA(winDir, MAX_PATH) == 0)
        std::cout << "Failed to obtain Windows directory.\n";
    else
        std::cout << "Windows directory:  " << winDir << "\n";

    if (GetTempPathA(MAX_PATH, tempDir) == 0)
        std::cout << "Failed to obtain temp directory.\n";
    else
        std::cout << "Temp directory:     " << tempDir << "\n";

    if (GetCurrentDirectoryA(MAX_PATH, currDir) == 0)
        std::cout << "Failed to obtain current directory.\n";
    else
        std::cout << "Current directory:  " << currDir << "\n";
}

// end of file
