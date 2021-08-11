#include "WindowHandler.h"

#include <JoyEngineContext.h>

bool WindowHandler::m_windowDestroyed = false;
JoyEngine::IWindowMessageHandler *WindowHandler::m_messageHandler = nullptr;