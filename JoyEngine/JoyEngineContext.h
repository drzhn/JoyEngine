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

    class JoyGraphicsContext : public IJoyGraphicsContext {
    public:
        JoyGraphicsContext(HINSTANCE instance, HWND windowHandle);

        ~JoyGraphicsContext();

        [[nodiscard]] HINSTANCE GetHINSTANCE() const noexcept override { return m_windowInstance; }

        [[nodiscard]] HWND GetHWND() const noexcept override { return m_windowHandle; }

        [[nodiscard]] Allocator *GetAllocator() const noexcept override { return m_allocator; }

        [[nodiscard]] VkInstance GetVkInstance() const noexcept override { return m_vkInstance; }

        [[nodiscard]] VkPhysicalDevice GetVkPhysicalDevice() const noexcept override { return m_physicalDevice; }

        [[nodiscard]] VkDevice GetVkDevice() const noexcept override { return m_logicalDevice; }

        [[nodiscard]] VkDebugUtilsMessengerEXT GetVkDebugUtilsMessengerEXT() const noexcept override { return m_debugMessenger; }

        [[nodiscard]] VkSurfaceKHR GetVkSurfaceKHR() const noexcept override { return m_surface; }

        [[nodiscard]] VkQueue GetGraphicsVkQueue() const noexcept override { return m_graphicsQueue; }

        [[nodiscard]] VkQueue GetPresentVkQueue() const noexcept override { return m_presentQueue; }

        [[nodiscard]]  VkCommandPool GetVkCommandPool() const noexcept override { return m_commandPool; }


    private:
        void CreateInstance();

        void SetupDebugMessenger();

        void CreateSurface();

        void PickPhysicalDevice();

        void CreateLogicalDevice();

        void CreateCommandPool();

    private:
        const std::vector<const char *> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        const char *validationLayerName = "VK_LAYER_KHRONOS_validation";

#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif
        HINSTANCE m_windowInstance;
        HWND m_windowHandle;
        Allocator *m_allocator;
        VkInstance m_vkInstance;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_logicalDevice;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        VkSurfaceKHR m_surface;
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
        VkCommandPool m_commandPool;
    };

    class JoyEngineContext : public IWindowMessageHandler {
    public:
        JoyEngineContext(HINSTANCE instance, HWND windowHandle) :
                m_windowHandle(windowHandle),
                m_graphicsContext(instance, windowHandle),
                m_memoryManager(),
                m_resourceManager(m_memoryManager, m_graphicsContext),
                m_sceneManager(),
                m_renderManager(m_graphicsContext) {
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
        JoyGraphicsContext m_graphicsContext;
        HWND m_windowHandle;
        MemoryManager m_memoryManager;
        ResourceManager m_resourceManager;
        SceneManager m_sceneManager;
        RenderManager m_renderManager;
    };
}


#endif//JOYENGINECONTEXT_H
