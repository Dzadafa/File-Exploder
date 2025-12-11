#pragma once
#include <windows.h>
#include "FileList.h"

class MainWindow {
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static int Run(HINSTANCE hInstance, int nCmdShow);

private:
    static FileList fileList;
    static HWND hStatusBar;
};
