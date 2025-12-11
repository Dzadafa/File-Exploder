#pragma once
#include <windows.h>
#include <string>
#include <commctrl.h>

class FileList {
private:
    HWND hListView;
    HINSTANCE hInst;
    std::wstring currentDirectory;

public:
    void Create(HWND parent, HINSTANCE instance);
    void Resize(int width, int height);
    void Load(const std::wstring& path);
    void Navigate(int index);
    
    HWND GetHandle() const { return hListView; }
    std::wstring GetCurrentPath() const { return currentDirectory; }
    int GetFileCount();
    int GetFolderCount();

private:
    void SetupImageList();
};
