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

    HANDLE hDir = CreateFileW(
        directory.c_str(),
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (hDir == INVALID_HANDLE_VALUE) return entries;

    const ULONG BUFFER_SIZE = 64 * 1024; 
    std::vector<BYTE> buffer(BUFFER_SIZE);

    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;
    bool firstCall = true;

    while (true) {
        status = NtQueryDirectoryFile(
            hDir, NULL, NULL, NULL, &ioStatus,
            buffer.data(), BUFFER_SIZE,
            (FILE_INFORMATION_CLASS)FileDirectoryInformation,
            FALSE,
            NULL,
            firstCall ? TRUE : FALSE
        );

        if (status == STATUS_NO_MORE_FILES) break;
        if (status != STATUS_SUCCESS) break;

        BYTE* p = buffer.data();
        while (true) {
            auto* info = (PMY_FILE_DIRECTORY_INFORMATION)p;

            std::wstring fileName(info->FileName, info->FileNameLength / sizeof(WCHAR));

            if (fileName != L"." && fileName != L"..") {
                FileEntry entry;
                entry.name = fileName;
                entry.attributes = info->FileAttributes;
                entry.isDirectory = (info->FileAttributes & FILE_ATTRIBUTE_DIRECTORY);
                
                entry.fullPath = directory;
                if (entry.fullPath.back() != L'\\') entry.fullPath += L"\\";
                entry.fullPath += fileName;

                entry.iconIndex = GetFileIconIndex(entry.fullPath, entry.attributes);
                entries.push_back(entry);
            }

            if (info->NextEntryOffset == 0) break;
            p += info->NextEntryOffset;
        }
        firstCall = false;
    }

    CloseHandle(hDir);
    return entries;
}
