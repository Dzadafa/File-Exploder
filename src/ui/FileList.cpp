#include "FileList.h"
#include "sys/FileSystem.h"
#include <shellapi.h>

void FileList::Create(HWND parent, HINSTANCE instance) {
    hInst = instance;
    RECT rcClient;
    GetClientRect(parent, &rcClient);

    hListView = CreateWindowExW(
        0, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS,
        0, 0, rcClient.right, rcClient.bottom,
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

void FileList::Resize(int width, int height) {
    if (hListView) MoveWindow(hListView, 0, 0, width, height, TRUE);
}

void FileList::Load(const std::wstring& path) {
    ListView_DeleteAllItems(hListView);
    currentDirectory = path;

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

void FileList::Navigate(int index) {
    wchar_t buffer[1024];
    ListView_GetItemText(hListView, index, 0, buffer, 1024);
    std::wstring itemText = buffer;

    if (itemText == L"[..]") {
        size_t lastSlash = currentDirectory.find_last_of(L"\\");
        if (lastSlash != std::wstring::npos && lastSlash > 2)
            currentDirectory = currentDirectory.substr(0, lastSlash);
        else 
            currentDirectory = currentDirectory.substr(0, 3);

        Load(currentDirectory);
        return;
    }

    std::wstring path = currentDirectory;
    if (path.back() != L'\\') path += L"\\";

    if (itemText.front() == L'[') {
        path += itemText.substr(1, itemText.length() - 2);
        Load(path);
    } else {
        path += itemText;
        ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}
