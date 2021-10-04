#ifndef JOY_ENGINE_H
#define JOY_ENGINE_H

#include "windows.h"

namespace JoyEngine {

    class GraphicsManager;

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

    class JoyEngine final: public IWindowMessageHandler {
    public:
        JoyEngine() = delete;

        JoyEngine(HINSTANCE instance, HWND windowHandle);

        void Init() const noexcept;

        void Start() const noexcept;

        void Update() const noexcept;

        void Stop() const noexcept;

        ~JoyEngine();

        void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    private:
        void InternalHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        HWND m_windowHandle;

        GraphicsManager *const m_graphicsContext = nullptr;
        MemoryManager *const m_memoryManager = nullptr;
        DataManager *const m_dataManager = nullptr;
        DescriptorSetManager *const m_descriptorSetManager = nullptr;
        ResourceManager *const m_resourceManager = nullptr;
        SceneManager *const m_sceneManager = nullptr;
        RenderManager *const m_renderManager = nullptr;
    };
}


#endif//JOY_ENGINE_H
