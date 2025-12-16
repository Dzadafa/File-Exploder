#include "MainWindow.h"
#include "sys/FileSystem.h"
#include <commctrl.h>
#include <objbase.h>
#include <vector>

FileList MainWindow::fileList;
History MainWindow::history;

HWND MainWindow::hStatusBar;
HWND MainWindow::hAddressBar;
HWND MainWindow::hDriveBox;
HWND MainWindow::hBtnBack;
HWND MainWindow::hBtnForward;

WNDPROC wpOldEditProc;

#define ID_BTN_BACK 5
#define ID_BTN_FWD 6

extern "C" int GetCoreVersion();

LRESULT CALLBACK AddressBarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_KEYDOWN:
            if (wParam == VK_RETURN) {
                wchar_t path[MAX_PATH];
                GetWindowTextW(hwnd, path, MAX_PATH);
                MainWindow::NavigateTo(path);
                return 0;
            }
            break;
        case WM_CHAR:
            if (wParam == VK_RETURN) return 0; 
            break;
    }
    return CallWindowProc(wpOldEditProc, hwnd, uMsg, wParam, lParam);
}

void MainWindow::RefreshDriveList() {
    SendMessage(hDriveBox, CB_RESETCONTENT, 0, 0);
    auto drives = FileSystem::GetDrives();
    for (const auto& drive : drives) {
        SendMessageW(hDriveBox, CB_ADDSTRING, 0, (LPARAM)drive.c_str());
    }
    SendMessage(hDriveBox, CB_SETCURSEL, 0, 0);
}

void MainWindow::UpdateButtons() {
    EnableWindow(hBtnBack, history.CanGoBack());
    EnableWindow(hBtnForward, history.CanGoForward());
}

void MainWindow::NavigateTo(const std::wstring& path) {
    history.Visit(path);
    fileList.Load(path);
    UpdateButtons();
    SetFocus(fileList.GetHandle());
}

void MainWindow::NavigateBack() {
    if (history.CanGoBack()) {
        std::wstring path = history.GoBack();
        fileList.Load(path);
        UpdateButtons();
    }
}

void MainWindow::NavigateForward() {
    if (history.CanGoForward()) {
        std::wstring path = history.GoForward();
        fileList.Load(path);
        UpdateButtons();
    }
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hBtnBack = CreateWindowW(L"BUTTON", L"<", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 0, 0, hwnd, (HMENU)ID_BTN_BACK, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hBtnForward = CreateWindowW(L"BUTTON", L">", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 0, 0, hwnd, (HMENU)ID_BTN_FWD, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hDriveBox = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                0, 0, 0, 0, hwnd, (HMENU)4, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            RefreshDriveList();

            hAddressBar = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0,
                hwnd, (HMENU)3, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            wpOldEditProc = (WNDPROC)SetWindowLongPtrW(hAddressBar, GWLP_WNDPROC, (LONG_PTR)AddressBarProc);

            hStatusBar = CreateWindowExW(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                0, 0, 0, 0, hwnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            
            fileList.Create(hwnd, ((LPCREATESTRUCT)lParam)->hInstance, hAddressBar);

            wchar_t buffer[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, buffer);
            history.Initialize(buffer);
            fileList.Load(buffer);
            UpdateButtons();
            
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            SendMessage(hStatusBar, WM_SIZE, 0, 0);
            RECT rcStatus; GetWindowRect(hStatusBar, &rcStatus);
            int statusH = rcStatus.bottom - rcStatus.top;

            int topBarH = 25;
            int padding = 2;
            int btnW = 30;
            int driveW = 60;

            if (hBtnBack) MoveWindow(hBtnBack, padding, padding, btnW, topBarH, TRUE);
            if (hBtnForward) MoveWindow(hBtnForward, padding + btnW + padding, padding, btnW, topBarH, TRUE);
            
            int driveX = padding + btnW + padding + btnW + padding;
            if (hDriveBox) MoveWindow(hDriveBox, driveX, padding, driveW, 200, TRUE);

            int addrX = driveX + driveW + padding;
            if (hAddressBar) MoveWindow(hAddressBar, addrX, padding, width - addrX - padding, topBarH, TRUE);

            fileList.Resize(0, topBarH + (padding * 2), width, height - statusH - (topBarH + padding * 2));
            return 0;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            
            if (id == ID_BTN_BACK) NavigateBack();
            else if (id == ID_BTN_FWD) NavigateForward();
            
            else if (id == 4 && HIWORD(wParam) == CBN_SELCHANGE) {
                int index = SendMessage(hDriveBox, CB_GETCURSEL, 0, 0);
                if (index != CB_ERR) {
                    wchar_t drive[10];
                    SendMessageW(hDriveBox, CB_GETLBTEXT, index, (LPARAM)drive);
                    NavigateTo(drive);
                }
            }
            return 0;
        }

        case WM_NOTIFY: {
            LPNMHDR lpnmh = (LPNMHDR)lParam;
            if (lpnmh->hwndFrom == fileList.GetHandle()) {
                if (lpnmh->code == NM_DBLCLK) {
                    LPNMITEMACTIVATE item = (LPNMITEMACTIVATE)lParam;
                    if (item->iItem != -1) {
                        std::wstring target = fileList.GetTarget(item->iItem);
                        DWORD attrs = GetFileAttributesW(target.c_str());
                        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                            NavigateTo(target);
                        } else {
                            ShellExecuteW(NULL, L"open", target.c_str(), NULL, NULL, SW_SHOWNORMAL);
                        }
                    }
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
