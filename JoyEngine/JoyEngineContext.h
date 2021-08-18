#ifndef JOYGRAPHICSCONTEXT_H
#define JOYGRAPHICSCONTEXT_H

#include <iostream>
#include "windows.h"

#include "SceneManager/SceneManager.h"
#include "RenderManager/RenderManager.h"
#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/ResourceManager.h"
#include "JoyGraphicsContext.h"

namespace JoyEngine {

    class MemoryManager;
    class ResourceManager;
    class SceneManager;
    class RenderManager;

    class IWindowMessageHandler {
    public:
        virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    };

    class JoyEngineContext : public IWindowMessageHandler {
    public:
        JoyEngineContext(HINSTANCE instance, HWND windowHandle) :
                m_windowHandle(windowHandle),
                m_graphicsContext(instance, windowHandle),
                m_memoryManager(),
                m_resourceManager(m_memoryManager, m_graphicsContext),
                m_sceneManager(),
                m_renderManager(m_graphicsContext, m_resourceManager) {
        }

        void Init() {
            m_memoryManager.Init();
            m_sceneManager.Init();
            m_renderManager.Init();
        }

        void Start() {
            m_memoryManager.Start();
            m_renderManager.Start();
        }

        void Update() {
            m_renderManager.Update();
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
        JoyGraphicsContext m_graphicsContext;
        HWND m_windowHandle;
        MemoryManager m_memoryManager;
        ResourceManager m_resourceManager;
        SceneManager m_sceneManager;
        RenderManager m_renderManager;
    };
}


#endif//JOYGRAPHICSCONTEXT_H
