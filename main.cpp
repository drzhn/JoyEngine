#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

#include "WindowHandler.h"
#include "JoyEngineContext.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <map>
#include <fstream>
#include <thread>
#include <atomic>
#include <memory>


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = &WindowHandler::WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    RECT wr = {0, 0, 512, 512};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            CLASS_NAME,                     // Window class
            L"",    // Window text
            WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,            // Window style
            // Size and position
            100, 100,
            wr.right - wr.left,  // width
            wr.bottom - wr.top,

            nullptr,       // Parent window
            nullptr,       // Menu
            hInstance,  // Instance handle
            nullptr        // Additional application data
    );

    if (hwnd == nullptr) {
        return 0;
    }
    JoyEngine::JoyEngineContext *graphicsContext = new JoyEngine::JoyEngineContext(hInstance, hwnd);
    graphicsContext->Init();

    WindowHandler::RegisterMessageHandler(graphicsContext, hwnd);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};

    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (WindowHandler::GetWindowDestroyed()) {
            break;
        }
        graphicsContext->Update();
    }

    delete graphicsContext;

    return 0;
}










