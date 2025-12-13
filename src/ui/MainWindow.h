#pragma once
#include <windows.h>
#include <string>
#include "FileList.h"

class MainWindow {
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static int Run(HINSTANCE hInstance, int nCmdShow);
    static void LoadPath(const std::wstring& path);

private:
    static FileList fileList;
    static HWND hStatusBar;
    static HWND hAddressBar;
    static HWND hDriveBox; 

    static void RefreshDriveList();
};
