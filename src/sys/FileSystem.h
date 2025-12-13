#pragma once
#include <string>
#include <vector>
#include <windows.h>

struct FileEntry {
    std::wstring name;
    std::wstring fullPath;
    bool isDirectory;
    int iconIndex;
    DWORD attributes;
};

class FileSystem {
public:
    static std::vector<FileEntry> ScanDirectory(const std::wstring& directory);
    static int GetFileIconIndex(const std::wstring& path, DWORD attributes);

    static std::vector<std::wstring> GetDrives();
};
