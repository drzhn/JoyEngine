#ifndef WINDOW_HANDLER_H
#define WINDOW_HANDLER_H

#include "windows.h"
#include "JoyEngine/JoyContext.h"

class WindowHandler {
public :
    static void RegisterMessageHandler(JoyEngine::IWindowMessageHandler *messageHandler, HWND hwnd) {
        m_messageHandler = messageHandler;
        m_hwnd = hwnd;
    }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_DESTROY: {
                m_windowDestroyed = true;
                PostQuitMessage(0);
                break;
            }
            default: {
                if (m_messageHandler != nullptr && hwnd == m_hwnd) {
                    m_messageHandler->HandleMessage(hwnd, uMsg, wParam, lParam);
                }
                break;
            }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    static bool GetWindowDestroyed() { return m_windowDestroyed; }

private :
    static bool m_windowDestroyed;
    static JoyEngine::IWindowMessageHandler *m_messageHandler;
    static HWND m_hwnd;
};

#endif // WINDOW_HANDLER_H