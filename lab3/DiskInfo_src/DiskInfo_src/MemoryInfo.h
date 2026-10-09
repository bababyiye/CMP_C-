// MemoryInfo.h
// Purpose: declares the function that queries and prints information
//          about the system RAM and virtual memory (task 5 of the lab).
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

// Queries system memory status via GlobalMemoryStatusEx and prints:
// RAM load (%), total/free physical memory, total/free page file,
// total/free virtual address space.
void PrintMemoryInfo();

// end of file
