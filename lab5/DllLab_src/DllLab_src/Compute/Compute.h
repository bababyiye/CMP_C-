// Compute.h
// Purpose: declares the Compute class, the concrete implementation of
//          ICompute that lives inside Compute.dll (task 1, "Compute.dll").
//          Compute.dll does not need to link against Interface.dll: it
//          only includes the shared Interface.h header to see the
//          ICompute definition, so that Compute's vtable layout matches
//          what MainApp.exe expects.
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once
#include "../Interface/Interface.h"
#include <vector>

class Compute : public ICompute
{
public:
    Compute() = default;

    void SetData(const std::vector<int>& data) override;
    void Run(int threadsCount) override;
    long long GetResult() override;
    const char* GetVersion() override;
    void Release() override;

private:
    ~Compute() override = default;

    // Counts how many values in data[begin, end) are prime, using plain
    // trial division (the "full enumeration" algorithm required by the
    // lab: every candidate divisor up to sqrt(n) is checked one by one).
    static long long CountPrimesInRange(const std::vector<int>& data, size_t begin, size_t end);

    std::vector<int> m_data;
    long long m_result = 0;

    // Artificial memory load (task 4.1): a scratch buffer allocated and
    // touched during Run(), sized proportionally to the input data, so
    // that the working-set graph recorded by MainApp.exe's resource
    // monitor visibly rises while the computation is in progress.
    std::vector<char> m_memoryLoadBuffer;
};

// Factory function exported by Compute.dll. MainApp.exe obtains a pointer
// to this function at run time via GetProcAddress (task 2.2) rather than
// linking against it directly, which is what makes the loading of
// Compute.dll fully dynamic.
extern "C" __declspec(dllexport) ICompute* CreateCompute();

// end of file
