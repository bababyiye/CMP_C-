// InterfaceLib.cpp
// Purpose: source file of Interface.dll. Defines BUILDING_INTERFACE_DLL
//          before including Interface.h so that INTERFACE_API expands to
//          __declspec(dllexport) for this translation unit, and provides
//          the single function this DLL exports.
// Author:  Zinchenko Maksym, group IPZ-23-5
#define BUILDING_INTERFACE_DLL
#include "Interface.h"

extern "C" INTERFACE_API const char* GetInterfaceVersion()
{
    return "Interface.dll v1.0 (ICompute abstract interface)";
}

// end of file
