#include "FileList.h"
#include "sys/FileSystem.h"
#include <shellapi.h>
#include <vector>
#include <stdio.h>

void FileList::Create(HWND parent, HINSTANCE instance, HWND addressBar) {
    hInst = instance;
    hAddressBar = addressBar; 

    hListView = CreateWindowExW(
        0, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS,
        0, 0, 0, 0, 

        parent, (HMENU)1, hInst, NULL
    );

    SetupImageList();

    LVCOLUMNW lvCol = {0};
    lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvCol.cx = 600;
    lvCol.pszText = const_cast<LPWSTR>(L"Name");
    ListView_InsertColumn(hListView, 0, &lvCol);
}

void FileList::SetupImageList() {
    SHFILEINFOW sfi = {0};
    HIMAGELIST hSystemImageList = (HIMAGELIST)SHGetFileInfoW(
        L"C:\\", 0, &sfi, sizeof(SHFILEINFOW), 
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON
    );
    ListView_SetImageList(hListView, hSystemImageList, LVSIL_SMALL);
}

void FileList::Resize(int x, int y, int width, int height) {
    if (hListView) MoveWindow(hListView, x, y, width, height, TRUE);
}

std::wstring FileList::GetItemText(int index) {
    wchar_t buffer[1024];
    ListView_GetItemText(hListView, index, 0, buffer, 1024);
    return std::wstring(buffer);
}

std::wstring FileList::GetPathFromItem(int index) {
    if (index == -1) return currentDirectory;

    std::wstring filename = GetItemText(index);
    if (filename == L"[..]") {
        size_t lastSlash = currentDirectory.find_last_of(L"\\");
        if (lastSlash != std::wstring::npos && lastSlash > 2)
            return currentDirectory.substr(0, lastSlash);
        return currentDirectory.substr(0, 3);
    }

    std::wstring fullPath = currentDirectory;
    if (fullPath.back() != L'\\') fullPath += L"\\";

    if (filename.front() == L'[') {
        fullPath += filename.substr(1, filename.length() - 2);
    } else {
        fullPath += filename;
    }
    return fullPath;
}

void FileList::Load(const std::wstring& path) {
    ListView_DeleteAllItems(hListView);
    currentDirectory = path;

    if (hAddressBar) {
        SetWindowTextW(hAddressBar, currentDirectory.c_str());
    }

    int index = 0;
    if (path.length() > 3) {
        LVITEMW lvItem = {0};
        lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
        lvItem.iItem = index++;
        lvItem.pszText = const_cast<LPWSTR>(L"[..]");
        lvItem.iImage = FileSystem::GetFileIconIndex(L"dummy", FILE_ATTRIBUTE_DIRECTORY);
        ListView_InsertItem(hListView, &lvItem);
    }

    auto files = FileSystem::ScanDirectory(path);

    for (const auto& file : files) {
        LVITEMW lvItem = {0};
        lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
        lvItem.iItem = index++;

        std::wstring displayName = file.name;
        if (file.isDirectory) displayName = L"[" + displayName + L"]";

        lvItem.pszText = const_cast<LPWSTR>(displayName.c_str());
        lvItem.iImage = file.iconIndex;

        ListView_InsertItem(hListView, &lvItem);
    }
}

void FileList::DeleteItem(int index) {
    std::wstring fullPath = GetPathFromItem(index);
    if (fullPath.empty()) return;

    fullPath.push_back(L'\0'); 

    SHFILEOPSTRUCTW fileOp = {0};
    fileOp.hwnd = hListView;
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = fullPath.c_str();
    fileOp.fFlags = FOF_ALLOWUNDO;

    if (SHFileOperationW(&fileOp) == 0 && !fileOp.fAnyOperationsAborted) {
        Load(currentDirectory);
    }
}

void FileList::ShowProperties(int index) {
    std::wstring fullPath = GetPathFromItem(index);
    if (fullPath.empty()) return;

    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (GetFileAttributesExW(fullPath.c_str(), GetFileExInfoStandard, &fileInfo)) {
        unsigned long long size = ((unsigned long long)fileInfo.nFileSizeHigh << 32) | fileInfo.nFileSizeLow;

        wchar_t msg[1024];
        swprintf(msg, 1024, L"File: %ls\n\nSize: %llu bytes\nAttributes: %lu", 
            fullPath.c_str(), size, fileInfo.dwFileAttributes);

        MessageBoxW(hListView, msg, L"Properties", MB_ICONINFORMATION);
    }
}

void FileList::OpenTerminal(int index) {
    std::wstring targetDir = currentDirectory;
    if (index != -1) {
        std::wstring itemPath = GetPathFromItem(index);
        DWORD attrs = GetFileAttributesW(itemPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            targetDir = itemPath;
        }
    }
    ShellExecuteW(NULL, L"open", L"cmd.exe", NULL, targetDir.c_str(), SW_SHOWNORMAL);
}

void FileList::OnRightClick() {
    POINT pt;
    GetCursorPos(&pt);
    POINT ptClient = pt;
    ScreenToClient(hListView, &ptClient);

    LVHITTESTINFO hitInfo = {0};
    hitInfo.pt = ptClient;
    ListView_HitTest(hListView, &hitInfo);

    HMENU hMenu = CreatePopupMenu();
    int itemClicked = hitInfo.iItem;

    if (itemClicked != -1) {
        ListView_SetItemState(hListView, itemClicked, 
            LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

        AppendMenuW(hMenu, MF_STRING, ACTION_OPEN, L"Open");
        AppendMenuW(hMenu, MF_STRING, ACTION_PROPERTIES, L"Properties");
        AppendMenuW(hMenu, MF_STRING, ACTION_TERMINAL, L"Open Terminal Here");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hMenu, MF_STRING, ACTION_DELETE, L"Delete");
    } else {
        AppendMenuW(hMenu, MF_STRING, ACTION_TERMINAL, L"Open Terminal Here");
    }

    int selection = TrackPopupMenu(hMenu, 
        TPM_RETURNCMD | TPM_RIGHTBUTTON, 
        pt.x, pt.y, 
        0, hListView, NULL
    );

    DestroyMenu(hMenu);

    switch (selection) {
        case ACTION_OPEN: Navigate(itemClicked); break;
        case ACTION_DELETE: DeleteItem(itemClicked); break;
        case ACTION_PROPERTIES: ShowProperties(itemClicked); break;
        case ACTION_TERMINAL: OpenTerminal(itemClicked); break;
    }
}

void FileList::Navigate(int index) {
    if (index == -1) return;
    std::wstring itemText = GetItemText(index);

    if (itemText == L"[..]") {
        size_t lastSlash = currentDirectory.find_last_of(L"\\");
        if (lastSlash != std::wstring::npos && lastSlash > 2)
            currentDirectory = currentDirectory.substr(0, lastSlash);
        else 
            currentDirectory = currentDirectory.substr(0, 3);
        Load(currentDirectory);
        return;
    }

    std::wstring path = GetPathFromItem(index);
    if (path.empty()) return;

    DWORD attrs = GetFileAttributesW(path.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        Load(path);
    } else {
        ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}
