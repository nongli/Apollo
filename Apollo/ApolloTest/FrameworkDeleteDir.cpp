#include <windows.h>
#include <conio.h>
#include "ApolloTestFramework.h"

using namespace std;

void ApolloTestFramework::DeleteDir(const char* dir) const {
    HANDLE      hFile;                      // Handle to directory
    string      strFilePath;                // Filepath
    string      strPattern;                 // Pattern
    string      directory = dir;
    WIN32_FIND_DATA FileInformation;        // File information

    strPattern = directory + "\\*.*";
    
    hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
    if (hFile != INVALID_HANDLE_VALUE) {
        do {
            if(FileInformation.cFileName[0] != '.') {
                strFilePath.erase();
                strFilePath = directory + "\\" + FileInformation.cFileName;
                // Delete file
               ::DeleteFile(strFilePath.c_str());
            }          
        } while(::FindNextFile(hFile, &FileInformation) == TRUE);

        // Close handle
        ::FindClose(hFile);
    }
}