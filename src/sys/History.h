#pragma once
#include <string>
#include <vector>

class History {
private:
    std::vector<std::wstring> backStack;
    std::vector<std::wstring> forwardStack;
    std::wstring currentPath;

public:
    void Initialize(const std::wstring& startPath);
    void Visit(const std::wstring& path);
    
    std::wstring GoBack();
    std::wstring GoForward();
    
    bool CanGoBack() const;
    bool CanGoForward() const;
    std::wstring GetCurrent() const { return currentPath; }
};
