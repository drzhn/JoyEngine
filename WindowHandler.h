#ifndef WINDOW_HANDLER_H
#define WINDOW_HANDLER_H

#include "windows.h"
#include "JoyEngine/JoyEngineContext.h"

class WindowHandler {
public :
    static void RegisterMessageHandler(JoyEngine::IWindowMessageHandler *messageHandler) {
        m_messageHandler = messageHandler;
    }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (m_messageHandler != nullptr) {
            m_messageHandler->HandleMessage(hwnd, uMsg, wParam, lParam);
        }
        if (uMsg == WM_DESTROY) {
            m_windowDestroyed = true;
            PostQuitMessage(0);
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    static bool GetWindowDestroyed() { return m_windowDestroyed; }

private :
    static bool m_windowDestroyed;
    static JoyEngine::IWindowMessageHandler *m_messageHandler;
};

#endif // WINDOW_HANDLER_H