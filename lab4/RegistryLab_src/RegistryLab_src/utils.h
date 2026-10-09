// utils.h
// Purpose: small helper functions shared by all modules of the Registry
//          lab utility (section printing, registry type names, hex
//          formatting of binary data).
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

#include <string>
#include <vector>
#include <windows.h>

// Prints a horizontal separator line and a section title to stdout,
// used to keep the console output readable.
void PrintSectionTitle(const std::string& title);

// Converts a REG_* type code (as returned by RegEnumValue /
// RegQueryValueEx) into its readable name, e.g. REG_SZ -> "REG_SZ".
std::string RegistryTypeToString(DWORD type);

// Converts a block of raw bytes into a comma-separated hexadecimal
// string, e.g. {0x01, 0xAB} -> "01,ab". Used both for console output
// and for writing REG_BINARY values into .reg files.
std::string BytesToHex(const BYTE* data, DWORD size);

// Doubles every backslash and quote in a string so it can be safely
// placed between double quotes inside a .reg file, e.g.
// C:\Temp -> C:\\Temp
std::string EscapeForRegFile(const std::string& input);

// end of file
