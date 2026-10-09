// Watcher.h
// Purpose: declares the directory-change monitoring function (task 9).
//          Implemented with the "next level" API - ReadDirectoryChangesW -
//          so that the name of the changed file is available directly,
//          without an extra call to re-scan the directory.
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

#include <string>
#include <windows.h>

// Watches the given directory for changes and appends every detected
// event (timestamp, event type, file name) to the given log file.
// Monitoring runs until the user presses ENTER in the console.
//
// notifyFilter selects which kinds of changes are reported; a
// combination of FILE_NOTIFY_CHANGE_* flags (file name, directory name,
// last write time, attributes, size, ...). watchSubtree, when true,
// also monitors all subdirectories of the given directory.
void WatchDirectory(const std::string& directoryPath,
                     const std::string& logFilePath,
                     DWORD notifyFilter,
                     bool watchSubtree = false);

// end of file
