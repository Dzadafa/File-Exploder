#include "FileSystem.h"
#include <shellapi.h>

int FileSystem::GetFileIconIndex(const std::wstring& path, DWORD attributes) {
    SHFILEINFOW sfi = {0};
    UINT flags = SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES;
    SHGetFileInfoW(path.c_str(), attributes, &sfi, sizeof(sfi), flags);
    return sfi.iIcon;
}

std::vector<FileEntry> FileSystem::ScanDirectory(const std::wstring& directory) {
    std::vector<FileEntry> entries;
    WIN32_FIND_DATAW findData;
    std::wstring searchPath = directory + L"\\*";

    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return entries;

    do {

        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) continue;

        FileEntry entry;
        entry.name = findData.cFileName;
        entry.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        entry.attributes = findData.dwFileAttributes;
        entry.fullPath = directory + L"\\" + entry.name;

        entry.iconIndex = GetFileIconIndex(entry.fullPath, entry.attributes);

        entries.push_back(entry);

    } while (FindNextFileW(hFind, &findData) != 0);

    FindClose(hFind);
    return entries;
}
