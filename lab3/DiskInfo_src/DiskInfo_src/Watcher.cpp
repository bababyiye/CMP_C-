// Watcher.cpp
// Purpose: implementation of directory-change monitoring (task 9) using
//          the asynchronous ReadDirectoryChangesW API, which - unlike
//          FindFirstChangeNotification - returns the name of the changed
//          file directly in its result buffer.
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "Watcher.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <vector>

namespace
{
    // Converts one FILE_ACTION_* code into a readable event name.
    std::string ActionToString(DWORD action)
    {
        switch (action)
        {
            case FILE_ACTION_ADDED:            return "File added";
            case FILE_ACTION_REMOVED:          return "File removed";
            case FILE_ACTION_MODIFIED:         return "File modified";
            case FILE_ACTION_RENAMED_OLD_NAME: return "Renamed (old name)";
            case FILE_ACTION_RENAMED_NEW_NAME: return "Renamed (new name)";
            default:                           return "Unknown action";
        }
    }

    // Converts a wide-character file name (as returned by
    // FILE_NOTIFY_INFORMATION) into a narrow std::string for logging.
    std::string WideToNarrow(const wchar_t* data, DWORD lengthBytes)
    {
        int charCount = lengthBytes / sizeof(WCHAR);
        int needed = WideCharToMultiByte(CP_UTF8, 0, data, charCount, nullptr, 0, nullptr, nullptr);
        std::string result(needed, '\0');
        WideCharToMultiByte(CP_UTF8, 0, data, charCount, &result[0], needed, nullptr, nullptr);
        return result;
    }

    // Background thread body: waits for the user to press ENTER and then
    // signals the exit event so the monitoring loop can stop cleanly.
    void WaitForEnterKey(HANDLE exitEvent, std::atomic<bool>& running)
    {
        std::cin.get();
        running = false;
        SetEvent(exitEvent);
    }
}

void WatchDirectory(const std::string& directoryPath,
                     const std::string& logFilePath,
                     DWORD notifyFilter,
                     bool watchSubtree)
{
    PrintSectionTitle("DIRECTORY CHANGE MONITORING (task 9)");

    HANDLE hDir = CreateFileA(
        directoryPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr);

    if (hDir == INVALID_HANDLE_VALUE)
    {
        std::cout << "Failed to open directory \"" << directoryPath
                  << "\". Error code: " << GetLastError() << "\n";
        return;
    }

    std::ofstream log(logFilePath, std::ios::app);
    if (!log.is_open())
    {
        std::cout << "Failed to open log file \"" << logFilePath << "\".\n";
        CloseHandle(hDir);
        return;
    }

    log << "----- Monitoring session started: " << GetTimestamp()
        << " | directory: " << directoryPath << " -----\n";
    log.flush();

    std::cout << "Watching \"" << directoryPath << "\" for changes.\n";
    std::cout << "Every detected event is written to \"" << logFilePath << "\".\n";
    std::cout << "Press ENTER to stop monitoring.\n";

    // Event used by the OVERLAPPED structure to signal I/O completion,
    // and a second event used to signal that the user asked to stop.
    HANDLE changeEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    HANDLE exitEvent   = CreateEventA(nullptr, TRUE, FALSE, nullptr);

    std::atomic<bool> running(true);
    std::thread inputThread(WaitForEnterKey, exitEvent, std::ref(running));

    const DWORD bufferSize = 8192;
    std::vector<BYTE> buffer(bufferSize);

    while (running)
    {
        OVERLAPPED overlapped = {};
        overlapped.hEvent = changeEvent;
        ResetEvent(changeEvent);

        DWORD bytesReturned = 0;
        BOOL ok = ReadDirectoryChangesW(
            hDir,
            buffer.data(),
            bufferSize,
            watchSubtree,
            notifyFilter,
            &bytesReturned,
            &overlapped,
            nullptr);

        if (!ok)
        {
            std::cout << "ReadDirectoryChangesW failed. Error code: "
                      << GetLastError() << "\n";
            break;
        }

        HANDLE waitHandles[2] = { changeEvent, exitEvent };
        DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

        if (waitResult == WAIT_OBJECT_0 + 1)
        {
            // Exit event was signalled by the input thread.
            break;
        }

        if (waitResult != WAIT_OBJECT_0)
        {
            std::cout << "Wait failed. Error code: " << GetLastError() << "\n";
            break;
        }

        DWORD transferred = 0;
        if (!GetOverlappedResult(hDir, &overlapped, &transferred, FALSE) || transferred == 0)
        {
            continue;
        }

        // The buffer may contain several FILE_NOTIFY_INFORMATION records
        // back to back; walk them using NextEntryOffset.
        BYTE* recordPtr = buffer.data();
        for (;;)
        {
            FILE_NOTIFY_INFORMATION* info =
                reinterpret_cast<FILE_NOTIFY_INFORMATION*>(recordPtr);

            std::string fileName = WideToNarrow(info->FileName, info->FileNameLength);
            std::string eventName = ActionToString(info->Action);
            std::string timestamp = GetTimestamp();

            std::cout << "[" << timestamp << "] " << eventName << ": " << fileName << "\n";
            log << "[" << timestamp << "] " << eventName << ": " << fileName << "\n";
            log.flush();

            if (info->NextEntryOffset == 0)
                break;

            recordPtr += info->NextEntryOffset;
        }
    }

    // Make sure the background thread has finished before returning.
    if (inputThread.joinable())
    {
        // If the loop stopped for a reason other than the user pressing
        // ENTER, the input thread might still be blocked on std::cin.get();
        // detaching avoids blocking the caller in that case.
        if (running)
            inputThread.detach();
        else
            inputThread.join();
    }

    log << "----- Monitoring session ended: " << GetTimestamp() << " -----\n\n";
    log.close();

    CloseHandle(changeEvent);
    CloseHandle(exitEvent);
    CloseHandle(hDir);

    std::cout << "Monitoring stopped.\n";
}

// end of file
