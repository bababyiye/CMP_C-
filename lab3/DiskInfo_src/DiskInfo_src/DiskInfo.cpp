// DiskInfo.cpp
// Purpose: implementation of logical drive enumeration, drive type
//          detection, file system information and free space
//          reporting (tasks 1-4 of the lab).
// Author:  Zinchenko Maksym, group IPZ-23-5
#include "DiskInfo.h"
#include "utils.h"
#include <windows.h>
#include <iostream>
#include <iomanip>

std::vector<std::string> GetLogicalDriveList()
{
    std::vector<std::string> drives;

    // First call with a large enough static buffer. GetLogicalDriveStrings
    // returns a multi-string: drive paths separated by '\0' and terminated
    // by an additional '\0' (double null at the end).
    char buffer[1024];
    DWORD length = GetLogicalDriveStringsA(sizeof(buffer), buffer);

    if (length == 0 || length > sizeof(buffer))
    {
        std::cout << "Failed to obtain logical drive strings. Error code: "
                  << GetLastError() << "\n";
        return drives;
    }

    char* ptr = buffer;
    while (*ptr != '\0')
    {
        std::string drive(ptr);
        drives.push_back(drive);
        ptr += drive.size() + 1; // skip drive string and its null terminator
    }

    return drives;
}

std::string DriveTypeToString(unsigned int driveType)
{
    switch (driveType)
    {
        case DRIVE_UNKNOWN:     return "Unknown";
        case DRIVE_NO_ROOT_DIR: return "No root directory";
        case DRIVE_REMOVABLE:   return "Removable (USB flash, SD card)";
        case DRIVE_FIXED:       return "Fixed disk (HDD/SSD)";
        case DRIVE_REMOTE:      return "Network drive";
        case DRIVE_CDROM:       return "CD/DVD drive";
        case DRIVE_RAMDISK:     return "RAM disk";
        default:                return "Undefined";
    }
}

std::string DriveTypeExplanation(unsigned int driveType)
{
    switch (driveType)
    {
        case DRIVE_UNKNOWN:
            return "The system cannot determine the drive type.";
        case DRIVE_NO_ROOT_DIR:
            return "The root path is invalid, e.g. there is no volume mounted at this path.";
        case DRIVE_REMOVABLE:
            return "A removable storage device, such as a USB flash drive or an SD card reader.";
        case DRIVE_FIXED:
            return "A non-removable internal storage device: a classic HDD or an SSD.";
        case DRIVE_REMOTE:
            return "A shared network resource (a mapped network drive, e.g. \\\\server\\share).";
        case DRIVE_CDROM:
            return "An optical drive used for CD or DVD media.";
        case DRIVE_RAMDISK:
            return "A virtual disk created directly in RAM; very fast but not persistent after reboot.";
        default:
            return "No explanation available for this drive type.";
    }
}

void PrintLogicalDrives(const std::vector<std::string>& drives)
{
    PrintSectionTitle("LOGICAL DRIVES (task 1)");

    if (drives.empty())
    {
        std::cout << "No logical drives were found.\n";
        return;
    }

    for (const auto& drive : drives)
    {
        // A quick availability check: try to query its type; DRIVE_NO_ROOT_DIR
        // and DRIVE_UNKNOWN are treated as "not available".
        UINT type = GetDriveTypeA(drive.c_str());
        bool available = (type != DRIVE_NO_ROOT_DIR && type != DRIVE_UNKNOWN);

        std::cout << "Drive letter: " << drive.substr(0, 1)
                  << "   Path: " << drive
                  << "   Status: " << (available ? "available" : "not available") << "\n";
    }
}

void PrintDriveTypes(const std::vector<std::string>& drives)
{
    PrintSectionTitle("DRIVE TYPES (task 2)");

    for (const auto& drive : drives)
    {
        UINT type = GetDriveTypeA(drive.c_str());
        std::cout << drive << " -> " << DriveTypeToString(type) << "\n";
        std::cout << "    " << DriveTypeExplanation(type) << "\n";
    }
}

void PrintVolumeInformation(const std::vector<std::string>& drives)
{
    PrintSectionTitle("FILE SYSTEM INFORMATION (task 3)");

    for (const auto& drive : drives)
    {
        UINT type = GetDriveTypeA(drive.c_str());
        // Querying a CD-ROM drive with no media inserted, or an unavailable
        // network drive, would block or fail; skip drives that are not
        // ready to avoid stalling the whole utility.
        if (type == DRIVE_NO_ROOT_DIR || type == DRIVE_UNKNOWN)
        {
            std::cout << drive << " -> skipped (drive not ready)\n";
            continue;
        }

        char volumeName[MAX_PATH + 1]   = { 0 };
        char fileSystemName[MAX_PATH + 1] = { 0 };
        DWORD serialNumber = 0;
        DWORD maxComponentLength = 0;
        DWORD fileSystemFlags = 0;

        BOOL ok = GetVolumeInformationA(
            drive.c_str(),
            volumeName, sizeof(volumeName),
            &serialNumber,
            &maxComponentLength,
            &fileSystemFlags,
            fileSystemName, sizeof(fileSystemName));

        if (!ok)
        {
            std::cout << drive << " -> failed to read volume information. Error code: "
                      << GetLastError() << "\n";
            continue;
        }

        std::cout << drive << ":\n";
        std::cout << "    Volume label:          "
                  << (volumeName[0] ? volumeName : "(no label)") << "\n";
        std::cout << "    Serial number:         "
                  << std::hex << std::uppercase << serialNumber
                  << std::dec << std::nouppercase << "\n";
        std::cout << "    Max file name length:  " << maxComponentLength << "\n";
        std::cout << "    File system:           " << fileSystemName << "\n";
        std::cout << "    Feature flags:\n";
        std::cout << "        Compression supported:        "
                  << ((fileSystemFlags & FILE_FILE_COMPRESSION) ? "yes" : "no") << "\n";
        std::cout << "        NTFS encryption (EFS):        "
                  << ((fileSystemFlags & FILE_SUPPORTS_ENCRYPTION) ? "yes" : "no") << "\n";
        std::cout << "        Access control lists (ACL):   "
                  << ((fileSystemFlags & FILE_PERSISTENT_ACLS) ? "yes" : "no") << "\n";
        std::cout << "        Transactional NTFS journal:   "
                  << ((fileSystemFlags & FILE_SUPPORTS_TRANSACTIONS) ? "yes" : "no") << "\n";
        std::cout << "        Case sensitive search:        "
                  << ((fileSystemFlags & FILE_CASE_SENSITIVE_SEARCH) ? "yes" : "no") << "\n";
        std::cout << "        Unicode file names:           "
                  << ((fileSystemFlags & FILE_UNICODE_ON_DISK) ? "yes" : "no") << "\n";
        std::cout << "        Sparse files supported:       "
                  << ((fileSystemFlags & FILE_SUPPORTS_SPARSE_FILES) ? "yes" : "no") << "\n";
    }
}

void PrintDiskSpace(const std::vector<std::string>& drives)
{
    PrintSectionTitle("DISK SPACE USAGE (task 4)");

    for (const auto& drive : drives)
    {
        UINT type = GetDriveTypeA(drive.c_str());
        if (type == DRIVE_NO_ROOT_DIR || type == DRIVE_UNKNOWN)
        {
            std::cout << drive << " -> skipped (drive not ready)\n";
            continue;
        }

        ULARGE_INTEGER freeBytesAvailable;   // available to the calling user
        ULARGE_INTEGER totalBytes;           // total capacity of the volume
        ULARGE_INTEGER totalFreeBytes;       // total free space on the volume

        BOOL ok = GetDiskFreeSpaceExA(
            drive.c_str(),
            &freeBytesAvailable,
            &totalBytes,
            &totalFreeBytes);

        if (!ok)
        {
            std::cout << drive << " -> failed to read disk space. Error code: "
                      << GetLastError() << "\n";
            continue;
        }

        unsigned long long used = totalBytes.QuadPart - totalFreeBytes.QuadPart;

        std::cout << drive << ":\n";
        std::cout << "    Total size:       " << FormatBytes(totalBytes.QuadPart) << "\n";
        std::cout << "    Used space:       " << FormatBytes(used)
                  << " (" << FormatPercent(used, totalBytes.QuadPart) << ")\n";
        std::cout << "    Free space:       " << FormatBytes(totalFreeBytes.QuadPart)
                  << " (" << FormatPercent(totalFreeBytes.QuadPart, totalBytes.QuadPart) << ")\n";
        std::cout << "    Available to you: " << FormatBytes(freeBytesAvailable.QuadPart) << "\n";
    }
}

// end of file
