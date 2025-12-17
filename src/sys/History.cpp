#include "History.h"

void History::Initialize(const std::wstring& startPath) {
    currentPath = startPath;
    backStack.clear();
    forwardStack.clear();
}

void History::Visit(const std::wstring& path) {
    if (path == currentPath) return;

    if (!currentPath.empty()) {
        backStack.push_back(currentPath);
    }
    
    currentPath = path;
    forwardStack.clear();
}

std::wstring History::GoBack() {
    if (backStack.empty()) return currentPath;

    forwardStack.push_back(currentPath);
    currentPath = backStack.back();
    backStack.pop_back();

    return currentPath;
}

std::wstring History::GoForward() {
    if (forwardStack.empty()) return currentPath;

    backStack.push_back(currentPath);
    currentPath = forwardStack.back();
    forwardStack.pop_back();

    return currentPath;
}

bool History::CanGoBack() const {
    return !backStack.empty();
}

bool History::CanGoForward() const {
    return !forwardStack.empty();
}
