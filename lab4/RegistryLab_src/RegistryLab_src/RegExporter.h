// RegExporter.h
// Purpose: declares the function that recursively exports a registry
//          key and all its subkeys into a text file in the standard
//          .reg format (task 8, variant 1 - self-written exporter,
//          without calling reg.exe).
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

#include <string>
#include <windows.h>

// Exports the key rootName\subKey (e.g. root = HKEY_CURRENT_USER,
// rootName = "HKEY_CURRENT_USER", subKey = "Software\\MyKey") and all
// of its nested subkeys and values into outputFile, using the classic
// "Windows Registry Editor Version 5.00" text format produced by
// RegEdit's own Export command. Internally walks the key tree with
// RegEnumKeyEx and RegEnumValue. Returns true on success.
bool ExportKeyToRegFile(HKEY root,
                         const std::string& rootName,
                         const std::string& subKey,
                         const std::string& outputFile);

// end of file
