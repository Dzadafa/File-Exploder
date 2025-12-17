#pragma once
#include <windows.h>
#include <string>
#include "FileList.h"
#include "sys/History.h"

class MainWindow {
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static int Run(HINSTANCE hInstance, int nCmdShow);
    
    static void NavigateTo(const std::wstring& path);
    static void NavigateBack();
    static void NavigateForward();
    static void UpdateButtons();

private:
    static FileList fileList;
    static History history;
    
    static HWND hStatusBar;
    static HWND hAddressBar;
    static HWND hDriveBox;
    static HWND hBtnBack;
    static HWND hBtnForward;
    
    static void RefreshDriveList();
};
