#pragma once
#include <windows.h>
#include <string> // Add this for std::wstring
#include "FileList.h"

class MainWindow {
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static int Run(HINSTANCE hInstance, int nCmdShow);
    
    // New Helper
    static void LoadPath(const std::wstring& path);

private:
    static FileList fileList;
    static HWND hStatusBar;
    static HWND hAddressBar;
};
