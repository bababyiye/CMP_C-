// Interface.h
// Purpose: shared abstract interface ICompute, implemented by Compute.dll
//          and consumed by MainApp.exe. Also declares GetInterfaceVersion,
//          the one function that Interface.dll itself exports and that
//          MainApp.exe imports statically (task 1, "Interface.dll").
//          This header is included, unchanged, by all three modules of
//          the lab (Interface.dll, Compute.dll, MainApp.exe).
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

#include <vector>

// BUILDING_INTERFACE_DLL is defined only when this header is compiled as
// part of Interface.dll itself (see InterfaceLib.cpp). Everywhere else -
// in Compute.dll and in MainApp.exe - the symbol is imported instead.
#if defined(BUILDING_INTERFACE_DLL)
    #define INTERFACE_API __declspec(dllexport)
#else
    #define INTERFACE_API __declspec(dllimport)
#endif

// Generalized abstract interface for a long-running computation. Both
// Compute.dll (which implements it) and MainApp.exe (which only ever
// talks to a bare ICompute*, never to the concrete Compute class) include
// this definition, which is how the "interface" and "implementation" are
// kept in separate DLLs while still sharing a common C++ vtable layout.
class ICompute
{
public:
    // Supplies the input data set that Run() will process.
    virtual void SetData(const std::vector<int>& data) = 0;

    // Runs the computation using the given number of worker threads.
    // Blocks the calling thread until every worker has finished.
    virtual void Run(int threadsCount) = 0;

    // Returns the result of the most recent Run() call.
    virtual long long GetResult() = 0;

    // Returns a short human-readable name/version string identifying the
    // concrete implementation (e.g. which DLL and algorithm is in use).
    virtual const char* GetVersion() = 0;

    // Destroys the object. Used instead of a public destructor + delete,
    // because the object is allocated inside Compute.dll with that DLL's
    // own heap/allocator and must therefore also be freed inside
    // Compute.dll, not by MainApp.exe's CRT.
    virtual void Release() = 0;

protected:
    // Only Release() is allowed to destroy the object, so the destructor
    // itself is protected rather than public.
    virtual ~ICompute() = default;
};

// Function-pointer type matching the factory exported by Compute.dll
// (see Compute.h / Compute.cpp). MainApp.exe casts the result of
// GetProcAddress(hCompute, "CreateCompute") to this type.
typedef ICompute* (*CreateCompute_t)();

// Exported by Interface.dll; returns a short version string for the
// interface module itself. MainApp.exe calls this once at startup to
// demonstrate static linking against Interface.dll (task 2.1).
extern "C" INTERFACE_API const char* GetInterfaceVersion();

// end of file
