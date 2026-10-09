// utils.h
// Purpose: small helper functions shared by all modules of the DiskInfo
//          utility (byte formatting, percentage formatting, timestamps).
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

#include <string>
#include <windows.h>

// Converts a raw byte count (received e.g. from GetDiskFreeSpaceEx or
// GlobalMemoryStatus) into a human readable string with an automatically
// chosen unit (B, KB, MB, GB, TB).
std::string FormatBytes(unsigned long long bytes);

// Formats a "part / whole" pair as a percentage string with one digit
// after the decimal point, e.g. "42.7%".
std::string FormatPercent(unsigned long long part, unsigned long long whole);

// Returns the current local time formatted as "YYYY-MM-DD HH:MM:SS",
// used to timestamp log records produced by the directory watcher.
std::string GetTimestamp();

// Prints a horizontal separator line and a section title to stdout,
// used to keep the console output readable.
void PrintSectionTitle(const std::string& title);

// end of file
