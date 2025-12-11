#include "MainWindow.h"
#include <commctrl.h>

FileList MainWindow::fileList;
HWND MainWindow::hStatusBar;

extern "C" int GetCoreVersion();

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            fileList.Create(hwnd, ((LPCREATESTRUCT)lParam)->hInstance);

            hStatusBar = CreateWindowExW(0, STATUSCLASSNAME, NULL,
                WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                0, 0, 0, 0, hwnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            wchar_t buffer[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, buffer);
            fileList.Load(buffer);
            return 0;
        }

        case WM_SIZE: {
            SendMessage(hStatusBar, WM_SIZE, 0, 0);
            RECT rcStatus; GetWindowRect(hStatusBar, &rcStatus);
            int statusH = rcStatus.bottom - rcStatus.top;
            fileList.Resize(LOWORD(lParam), HIWORD(lParam) - statusH);
            return 0;
        }

        case WM_NOTIFY: {
            LPNMHDR lpnmh = (LPNMHDR)lParam;
            if (lpnmh->hwndFrom == fileList.GetHandle() && lpnmh->code == NM_DBLCLK) {
                LPNMITEMACTIVATE item = (LPNMITEMACTIVATE)lParam;
                if (item->iItem != -1) fileList.Navigate(item->iItem);
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
    std::wstring title = L"File Exploder";

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    CoUninitialize();
    return 0;
}
