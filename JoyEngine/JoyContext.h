#ifndef JOYGRAPHICSCONTEXT_H
#define JOYGRAPHICSCONTEXT_H

#include <iostream>
#include "windows.h"

#include "Utils/Assert.h"


namespace JoyEngine {

    class JoyGraphicsContext;

    class MemoryManager;

    class DataManager;

    class DescriptorSetManager;

    class ResourceManager;

    class SceneManager;

    class RenderManager;

    class IWindowMessageHandler {
    public:
        virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    };

    class JoyContext : public IWindowMessageHandler {
    public:
        JoyContext() = delete;

        JoyContext(HINSTANCE instance, HWND windowHandle);

        void Init();

        void Start();

        void Update();

        void Stop();

        ~JoyContext();

        void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

        static JoyGraphicsContext *Graphics() noexcept;

        static MemoryManager *Memory() noexcept;

        static DataManager *Data() noexcept;

        static DescriptorSetManager *DescriptorSet() noexcept;

        static ResourceManager *Resource() noexcept;

        static SceneManager *Scene() noexcept;

        static RenderManager *Render() noexcept;

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
