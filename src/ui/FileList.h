#pragma once
#include <windows.h>
#include <string>
#include <commctrl.h>

class FileList {
private:
    HWND hListView;
    HINSTANCE hInst;
    std::wstring currentDirectory;

    enum MenuActions {
        ACTION_OPEN = 1,
        ACTION_DELETE = 2,
        ACTION_PROPERTIES = 3
    };

public:
    void Create(HWND parent, HINSTANCE instance);
    void Resize(int width, int height);
    void Load(const std::wstring& path);
    void Navigate(int index);
    void OnRightClick();

    HWND GetHandle() const { return hListView; }
    std::wstring GetCurrentPath() const { return currentDirectory; }

private:
    void SetupImageList();
    std::wstring GetItemText(int index);
    std::wstring GetPathFromItem(int index);
    void DeleteItem(int index);
    void ShowProperties(int index);
};
