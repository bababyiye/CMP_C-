// ResourceMonitor.h
// Purpose: declares a background sampler that periodically records the
//          process's working-set memory and live thread count to a CSV
//          file while a benchmark phase is running (task 4.1 - the raw
//          data that the resource-usage graphs are later built from).
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

#include <string>
#include <thread>
#include <atomic>

class ResourceMonitor
{
public:
    // Starts a background sampling thread.
    // logFilePath - CSV file to append rows to; a header row is written
    //               automatically the first time the file is created.
    // phaseLabel  - text identifying which benchmark combination is
    //               currently running (e.g. "size=1000000;threads=4"),
    //               written into every sampled row so the CSV can later
    //               be filtered/grouped by phase in Excel.
    // intervalMs  - sampling period in milliseconds.
    void Start(const std::string& logFilePath, const std::string& phaseLabel, int intervalMs);

    // Stops the background thread and flushes/closes the file.
    void Stop();

    ~ResourceMonitor();

private:
    void SamplingLoop();

    std::thread m_thread;
    std::atomic<bool> m_running{ false };
    std::string m_logFilePath;
    std::string m_phaseLabel;
    int m_intervalMs = 50;
};

// end of file
