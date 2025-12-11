#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h> 

#include <string>
#include <vector>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

extern "C" int GetCoreVersion();
extern "C" int AsmComputeTotal(int files, int folders);

HWND hListView;
HWND hStatusBar;
HINSTANCE hInst;
std::wstring currentDirectory;

int GetFileIconIndex(const std::wstring& path, DWORD attributes) {
    SHFILEINFOW sfi = {0};

    UINT flags = SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES;

    SHGetFileInfoW(path.c_str(), attributes, &sfi, sizeof(sfi), flags);
    return sfi.iIcon;
}

void SetupImageList(HWND hListView) {

    SHFILEINFOW sfi = {0};
    HIMAGELIST hSystemImageList = (HIMAGELIST)SHGetFileInfoW(
        L"C:\\", 
        0, 
        &sfi, 
        sizeof(SHFILEINFOW), 
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON
    );

    ListView_SetImageList(hListView, hSystemImageList, LVSIL_SMALL);
}

void UpdateStatusBar(int files, int folders) {
    int total = AsmComputeTotal(files, folders);
    std::wstring statusMsg = L" Files: " + std::to_wstring(files) + 
                             L" | Folders: " + std::to_wstring(folders) + 
                             L" | Total: " + std::to_wstring(total);
    SendMessageW(hStatusBar, SB_SETTEXTW, 0, (LPARAM)statusMsg.c_str());
}

void AddItemToListView(const std::wstring& text, int index, int iconIndex) {
    LVITEMW lvItem = {0};
    lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 

    lvItem.iItem = index;
    lvItem.iSubItem = 0;
    lvItem.pszText = const_cast<LPWSTR>(text.c_str()); 
    lvItem.iImage = iconIndex; 

    ListView_InsertItem(hListView, &lvItem);
}

std::wstring GetItemText(int index) {
    wchar_t buffer[1024];
    ListView_GetItemText(hListView, index, 0, buffer, 1024);
    return std::wstring(buffer);
}

void LoadFiles(const std::wstring& directory) {
    ListView_DeleteAllItems(hListView);
    currentDirectory = directory;

    if (directory.length() > 3) { 

        int folderIcon = GetFileIconIndex(L"dummy_folder", FILE_ATTRIBUTE_DIRECTORY);
        AddItemToListView(L"[..]", 0, folderIcon);
    }

    WIN32_FIND_DATAW findData;
    std::wstring searchPath = directory + L"\\*";
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

    int index = (directory.length() > 3) ? 1 : 0;
    int countFiles = 0;
    int countFolders = 0;

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) continue;

            std::wstring name = findData.cFileName;
            std::wstring fullPath = directory + L"\\" + name;

            int iconIdx = GetFileIconIndex(fullPath, findData.dwFileAttributes);

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                countFolders++;
            } else {
                countFiles++;
            }

            AddItemToListView(name, index++, iconIdx);

        } while (FindNextFileW(hFind, &findData) != 0);
        FindClose(hFind);
    }

    UpdateStatusBar(countFiles, countFolders);
}

void Navigate(int itemIndex) {
    std::wstring itemText = GetItemText(itemIndex);
    if (itemText.empty()) return;

    if (itemText == L"[..]") {
        size_t lastSlash = currentDirectory.find_last_of(L"\\");
        if (lastSlash != std::wstring::npos && lastSlash > 2) {
            currentDirectory = currentDirectory.substr(0, lastSlash);
        } else if (lastSlash == 2) {
            currentDirectory = currentDirectory.substr(0, 3);
        }
        LoadFiles(currentDirectory);
        return;
    }

    std::wstring potentialPath = currentDirectory;
    if (potentialPath.back() != L'\\') potentialPath += L"\\";
    potentialPath += itemText;

    DWORD attrs = GetFileAttributesW(potentialPath.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
         LoadFiles(potentialPath);
    } else {
         ShellExecuteW(NULL, L"open", potentialPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}

void CreateControls(HWND hwndParent) {
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);

    hListView = CreateWindowExW(
        0, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, 

        0, 0, rcClient.right, rcClient.bottom - 20,
        hwndParent, (HMENU)1, hInst, NULL
    );

    SetupImageList(hListView);

    LVCOLUMNW lvCol = {0};
    lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvCol.cx = 600; 
    lvCol.pszText = const_cast<LPWSTR>(L"Name");
    ListView_InsertColumn(hListView, 0, &lvCol);

    hStatusBar = CreateWindowExW(
        0, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwndParent, (HMENU)2, hInst, NULL
    );
}

void ResizeControls(HWND hwnd, int width, int height) {
    if (hStatusBar) {
        SendMessage(hStatusBar, WM_SIZE, 0, 0);
        RECT rcStatus;
        GetWindowRect(hStatusBar, &rcStatus);
        int statusHeight = rcStatus.bottom - rcStatus.top;
        if (hListView) MoveWindow(hListView, 0, 0, width, height - statusHeight, TRUE);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            CreateControls(hwnd);
            wchar_t buffer[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, buffer);
            LoadFiles(buffer);
            return 0;
        }
        case WM_SIZE:
            ResizeControls(hwnd, LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_NOTIFY: {
            LPNMHDR lpnmh = (LPNMHDR)lParam;
            if (lpnmh->hwndFrom == hListView && lpnmh->code == NM_DBLCLK) {
                LPNMITEMACTIVATE lpnmItem = (LPNMITEMACTIVATE)lParam;
                if (lpnmItem->iItem != -1) Navigate(lpnmItem->iItem);
            }
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    hInst = hInstance;

    CoInitialize(NULL); 

    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES };
    InitCommonControlsEx(&icex);

    const wchar_t CLASS_NAME[] = L"FileManagerClass";
    WNDCLASSW wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"ASM/C++ File Manager",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
    return 0;
}
