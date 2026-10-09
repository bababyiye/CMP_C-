// UserInfo.h
// Purpose: declares functions that print the computer name, the current
//          user name and the main system directories (tasks 6, 7, 8).
// Author:  Zinchenko Maksym, group IPZ-23-5
#pragma once

// Prints the NetBIOS computer name (GetComputerName).
void PrintComputerName();

// Prints the name of the currently logged on user (GetUserName).
void PrintUserName();

// Prints the main system directories: Windows system directory,
// Windows directory, temporary directory and current working
// directory of the process (task 8).
void PrintSystemDirectories();

// end of file
