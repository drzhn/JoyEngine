#ifndef JOYGRAPHICSCONTEXT_H
#define JOYGRAPHICSCONTEXT_H

#include <iostream>
#include "windows.h"

#include "SceneManager/SceneManager.h"
#include "RenderManager/RenderManager.h"
#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/ResourceManager.h"
#include "DataManager/DataManager.h"
#include "ResourceManager/DescriptorSetManager.h"
#include "JoyGraphicsContext.h"
#include "Utils/Assert.h"


namespace JoyEngine {

    class MemoryManager;

    class ResourceManager;

    class SceneManager;

    class RenderManager;

    class IWindowMessageHandler {
    public:
        virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    };

    class JoyContext : public IWindowMessageHandler {
    public:
        JoyContext(HINSTANCE instance, HWND windowHandle);

        void Init();

        void Start();

        void Update();

        void Stop();

        ~JoyContext();

        void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

        static JoyGraphicsContext *const Graphics() noexcept;

        static MemoryManager *const Memory() noexcept;

        static DataManager *const Data() noexcept;

        static DescriptorSetManager *const DescriptorSet() noexcept;

        static ResourceManager *const Resource() noexcept;

        static SceneManager *const Scene() noexcept;

        static RenderManager *const Render() noexcept;

    private:
        void InternalHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        static JoyContext *m_instance;
        HWND m_windowHandle;

        JoyGraphicsContext *const m_graphicsContext = nullptr;
        MemoryManager *const m_memoryManager = nullptr;
        DataManager *const m_dataManager = nullptr;
        DescriptorSetManager *const m_descriptorSetManager = nullptr;
        ResourceManager *const m_resourceManager = nullptr;
        SceneManager *const m_sceneManager = nullptr;
        RenderManager *const m_renderManager = nullptr;
    };
}


#endif//JOYGRAPHICSCONTEXT_H
