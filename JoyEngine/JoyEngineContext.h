#ifndef JOYENGINECONTEXT_H
#define JOYENGINECONTEXT_H

#include <iostream>
#include "windows.h"

#include "SceneManager/SceneManager.h"
#include "RenderManager/RenderManager.h"
#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/ResourceManager.h"

namespace JoyEngine {

    class IWindowMessageHandler {
    public:
        virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    };

    class JoyEngineContext : public IWindowMessageHandler {
    public:
        JoyEngineContext(HINSTANCE instance, HWND windowHandle) :
                m_windowHandle(windowHandle),
                m_memoryManager(),
                m_resourceManager(m_memoryManager),
                m_sceneManager(),
                m_renderManager(instance, windowHandle) {
        }

        void Init() {
            m_memoryManager.Init();
            m_renderManager.Init();
            m_sceneManager.Init();
        }

        void Start() {
            m_memoryManager.Start();
            m_renderManager.Start();
        }

        void Update() {

        }

        void Stop() {
            m_renderManager.Stop(); // will destroy managers in reverse order
            m_memoryManager.Stop();
        }

        ~JoyEngineContext() {
            std::cout << "Context destroyed" << std::endl;
        }

        void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override {
            if (hwnd == m_windowHandle) {
                InternalHandleMessage(uMsg, wParam, lParam);
            }
        };

    private:
        void InternalHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
            if (uMsg == WM_DESTROY) {
                Stop();
            }
        }

    private:
        HWND m_windowHandle;
        MemoryManager m_memoryManager;
        ResourceManager m_resourceManager;
        SceneManager m_sceneManager;
        RenderManager m_renderManager;
    };
}


#endif//JOYENGINECONTEXT_H
