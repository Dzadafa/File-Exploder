#include "MainWindow.h"
#include "sys/FileSystem.h"
#include <commctrl.h>
#include <objbase.h>
#include <vector>

FileList MainWindow::fileList;
HWND MainWindow::hStatusBar;
HWND MainWindow::hAddressBar;
HWND MainWindow::hDriveBox;
WNDPROC wpOldEditProc;

extern "C" int GetCoreVersion();

LRESULT CALLBACK AddressBarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_KEYDOWN:
            if (wParam == VK_RETURN) {
                wchar_t path[MAX_PATH];
                GetWindowTextW(hwnd, path, MAX_PATH);
                MainWindow::LoadPath(path);
                return 0;
            }
            break;
        case WM_CHAR:
            if (wParam == VK_RETURN) return 0; 
            break;
    }
    return CallWindowProc(wpOldEditProc, hwnd, uMsg, wParam, lParam);
}

void MainWindow::LoadPath(const std::wstring& path) {
    fileList.Load(path);
    SetFocus(fileList.GetHandle());
}

void MainWindow::RefreshDriveList() {
    SendMessage(hDriveBox, CB_RESETCONTENT, 0, 0);
    auto drives = FileSystem::GetDrives();
    for (const auto& drive : drives) {
        SendMessageW(hDriveBox, CB_ADDSTRING, 0, (LPARAM)drive.c_str());
    }
    SendMessage(hDriveBox, CB_SETCURSEL, 0, 0);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hDriveBox = CreateWindowW(
                L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                0, 0, 0, 0, 
                hwnd, (HMENU)4, ((LPCREATESTRUCT)lParam)->hInstance, NULL
            );
            
            RefreshDriveList();

            hAddressBar = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 
                0, 0, 0, 0, hwnd, (HMENU)3, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            wpOldEditProc = (WNDPROC)SetWindowLongPtrW(hAddressBar, GWLP_WNDPROC, (LONG_PTR)AddressBarProc);

            hStatusBar = CreateWindowExW(0, STATUSCLASSNAME, NULL,
                WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                0, 0, 0, 0, hwnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            fileList.Create(hwnd, ((LPCREATESTRUCT)lParam)->hInstance, hAddressBar);

            wchar_t buffer[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, buffer);
            fileList.Load(buffer);
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            SendMessage(hStatusBar, WM_SIZE, 0, 0);
            RECT rcStatus; GetWindowRect(hStatusBar, &rcStatus);
            int statusH = rcStatus.bottom - rcStatus.top;

            int topBarH = 25;
            int driveBoxW = 60;
            int padding = 2;

            if (hDriveBox) 
                MoveWindow(hDriveBox, padding, padding, driveBoxW, 200, TRUE);

            if (hAddressBar) 
                MoveWindow(hAddressBar, driveBoxW + (padding * 2), padding, width - driveBoxW - (padding * 3), topBarH, TRUE);

            fileList.Resize(0, topBarH + (padding * 2), width, height - statusH - (topBarH + padding * 2));
            return 0;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == 4 && HIWORD(wParam) == CBN_SELCHANGE) {
                int index = SendMessage(hDriveBox, CB_GETCURSEL, 0, 0);
                if (index != CB_ERR) {
                    wchar_t drive[10];
                    SendMessageW(hDriveBox, CB_GETLBTEXT, index, (LPARAM)drive);
                    LoadPath(drive);
                }
                SetFocus(fileList.GetHandle()); 
            }
            return 0;
        }

        case WM_NOTIFY: {
            LPNMHDR lpnmh = (LPNMHDR)lParam;
            if (lpnmh->hwndFrom == fileList.GetHandle()) {
                if (lpnmh->code == NM_DBLCLK) {
                    LPNMITEMACTIVATE item = (LPNMITEMACTIVATE)lParam;
                    if (item->iItem != -1) fileList.Navigate(item->iItem);
                }
                else if (lpnmh->code == NM_RCLICK) {
                    fileList.OnRightClick();
                }
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int MainWindow::Run(HINSTANCE hInstance, int nCmdShow) {
    CoInitialize(NULL);
    INITCOMMONCONTROLSEX icex = {sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES};
    InitCommonControlsEx(&icex);

    const wchar_t CLASS_NAME[] = L"FileExploderClass";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    RegisterClassW(&wc);

    int asmVer = GetCoreVersion();
    std::wstring title = L"File Exploder (ASM v" + std::to_wstring(asmVer) + L")";

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);
    SetFocus(fileList.GetHandle());

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    CoUninitialize();
    return 0;
}
