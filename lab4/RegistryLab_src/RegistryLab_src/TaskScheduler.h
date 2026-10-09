// TaskScheduler.h
// Purpose: declares the function that lists Windows Task Scheduler
//          entries directly from the registry (task 7 of the lab,
//          without using the Task Scheduler COM API).
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

// Enumerates the subkeys of
//   HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Schedule\TaskCache\Tasks
// As allowed by the lab description, only the current user's hive
// (HKCU) is read, which avoids the extra step of resolving another
// user's SID. For every task subkey (named by a GUID) the function
// prints: the task ID (the subkey's GUID name), the task path stored
// in its "Path" value, and whether "Actions" / "Triggers" binary
// values are present (their raw size is shown, since their internal
// layout is an undocumented binary structure and is not decoded here).
void ListScheduledTasksFromRegistry();

// end of file
