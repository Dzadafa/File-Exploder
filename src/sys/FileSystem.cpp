#include "FileSystem.h"
#include <shellapi.h>
#include <vector>
#include <winternl.h>

typedef struct _MY_FILE_DIRECTORY_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    WCHAR FileName[1];
} MY_FILE_DIRECTORY_INFORMATION, *PMY_FILE_DIRECTORY_INFORMATION;

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define STATUS_NO_MORE_FILES ((NTSTATUS)0x80000006L)
#define FileDirectoryInformation 1

extern "C" {
    NTSTATUS NTAPI NtQueryDirectoryFile(
        HANDLE FileHandle,
        HANDLE Event,
        PIO_APC_ROUTINE ApcRoutine,
        PVOID ApcContext,
        PIO_STATUS_BLOCK IoStatusBlock,
        PVOID FileInformation,
        ULONG Length,
        FILE_INFORMATION_CLASS FileInformationClass,
        BOOLEAN ReturnSingleEntry,
        PUNICODE_STRING FileName,
        BOOLEAN RestartScan
    );
}

int FileSystem::GetFileIconIndex(const std::wstring& path, DWORD attributes) {
    SHFILEINFOW sfi = {0};
    UINT flags = SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES;
    SHGetFileInfoW(path.c_str(), attributes, &sfi, sizeof(sfi), flags);
    return sfi.iIcon;
}

std::vector<std::wstring> FileSystem::GetDrives() {
    std::vector<std::wstring> drives;
    DWORD mask = GetLogicalDrives(); 
    for (int i = 0; i < 26; ++i) {
        if (mask & (1 << i)) {
            wchar_t driveName[] = { (wchar_t)(L'A' + i), L':', L'\\', L'\0' };
            drives.push_back(driveName);
        }
    }
    return drives;
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
