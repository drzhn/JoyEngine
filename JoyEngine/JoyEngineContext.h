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
                m_graphicsContext(new JoyGraphicsContext(instance, windowHandle)),
                m_memoryManager(new MemoryManager(m_graphicsContext)),
                m_resourceManager(new ResourceManager(m_graphicsContext)),
                m_sceneManager(new SceneManager()),
                m_renderManager(new RenderManager(m_graphicsContext, m_resourceManager)) {
            assert(m_graphicsContext != nullptr);
            assert(m_memoryManager != nullptr);
            assert(m_resourceManager != nullptr);
            assert(m_sceneManager != nullptr);
            assert(m_renderManager != nullptr);
            std::cout << "Context created" << std::endl;
        }

        void Init() {
            m_memoryManager->Init();
            m_sceneManager->Init();
            m_renderManager->Init();
        }

        void Start() {
            m_memoryManager->Start();
            m_renderManager->Start();
        }

        void Update() {
            m_renderManager->Update();
        }

        void Stop() {
            m_renderManager->Stop(); // will destroy managers in reverse order
            m_memoryManager->Stop();
        }

        ~JoyEngineContext() {
            delete m_sceneManager; // unregister mesh renderers, remove descriptor set, pipelines, pipeline layouts
            delete m_resourceManager; //delete all scene render data (buffers, textures)
            delete m_renderManager; //delete swapchain, synchronisation, framebuffers
            delete m_memoryManager; //free gpu memory
            delete m_graphicsContext; //delete surface, device, instance
            std::cout << "Context destroyed" << std::endl;
        }

        void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override {
            InternalHandleMessage(uMsg, wParam, lParam);
        };

    private:
        void InternalHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {

        }

    private:
        HWND m_windowHandle;

        JoyGraphicsContext *const m_graphicsContext = nullptr;
        MemoryManager *const m_memoryManager = nullptr;
        ResourceManager *const m_resourceManager = nullptr;
        SceneManager *const m_sceneManager = nullptr;
        RenderManager *const m_renderManager = nullptr;
    };
}


#endif//JOYGRAPHICSCONTEXT_H
