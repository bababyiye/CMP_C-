// DiskInfo.h
// Purpose: declares functions that enumerate logical drives and print
//          their type, file system and free/used space (tasks 1-4).
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

#include <string>
#include <vector>

// Returns the list of logical drive root paths present in the system,
// e.g. {"C:\\", "D:\\", "E:\\"}. Internally uses GetLogicalDriveStrings.
std::vector<std::string> GetLogicalDriveList();

// Translates a GetDriveType() return code into a short human readable
// name (task 2), e.g. DRIVE_FIXED -> "Fixed disk (HDD/SSD)".
std::string DriveTypeToString(unsigned int driveType);

// Returns a one-line explanation of what the given drive type means
// and where it is typically used (task 2, "explain the type").
std::string DriveTypeExplanation(unsigned int driveType);

// Prints the list of logical drives together with their availability
// status (task 1).
void PrintLogicalDrives(const std::vector<std::string>& drives);

// Prints drive type and its explanation for every drive in the list
// (task 2).
void PrintDriveTypes(const std::vector<std::string>& drives);

// Prints file system information (label, serial number, max file name
// length, file system name, feature flags) for every drive in the list
// (task 3, GetVolumeInformation).
void PrintVolumeInformation(const std::vector<std::string>& drives);

// Prints total/free/available space in bytes, GB and percentage used
// for every drive in the list (task 4, GetDiskFreeSpaceEx).
void PrintDiskSpace(const std::vector<std::string>& drives);

// end of file
