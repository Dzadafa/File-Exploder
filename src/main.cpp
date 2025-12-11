#include <windows.h>
#include "ui/MainWindow.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    return MainWindow::Run(hInstance, nCmdShow);
}
