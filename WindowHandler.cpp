#include "WindowHandler.h"

#include <JoyEngine.h>

HWND WindowHandler::m_hwnd = nullptr;
bool WindowHandler::m_windowDestroyed = false;
JoyEngine::IWindowMessageHandler *WindowHandler::m_messageHandler = nullptr;