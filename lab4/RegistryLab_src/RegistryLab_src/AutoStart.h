// AutoStart.h
// Purpose: declares functions that enumerate and modify the Windows
//          autostart registry keys (tasks 4-6 of the lab).
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

#include <string>

// Enumerates the four standard autostart locations:
//   HKLM\Software\Microsoft\Windows\CurrentVersion\Run
//   HKLM\Software\Microsoft\Windows\CurrentVersion\RunOnce
//   HKCU\Software\Microsoft\Windows\CurrentVersion\Run
//   HKCU\Software\Microsoft\Windows\CurrentVersion\RunOnce
// and prints, for every value found: its name, the command/path stored
// in it and the source hive (HKLM or HKCU). Uses RegOpenKeyEx,
// RegEnumValue and RegCloseKey (task 4).
void ListAutoStartEntries();

// Adds (or overwrites) a REG_SZ value in
// HKCU\Software\Microsoft\Windows\CurrentVersion\Run
// using RegSetValueEx (task 5). valueName is the parameter name shown
// in the Run key (e.g. "MyAutoStartApp"), applicationPath is the full
// path that will be launched at logon.
bool AddAutoStartEntry(const std::string& valueName, const std::string& applicationPath);

// end of file
