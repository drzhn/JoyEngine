#include "JoyContext.h"

#include "SceneManager/SceneManager.h"
#include "RenderManager/RenderManager.h"
#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/ResourceManager.h"
#include "DataManager/DataManager.h"
#include "ResourceManager/DescriptorSetManager.h"
#include "JoyGraphicsContext.h"

namespace JoyEngine {

    JoyContext *JoyContext::m_instance = nullptr;

    JoyContext::JoyContext(HINSTANCE instance, HWND windowHandle) :
            m_windowHandle(windowHandle),
            m_graphicsContext(new JoyGraphicsContext(instance, windowHandle)),
            m_memoryManager(new MemoryManager()),
            m_dataManager(new DataManager()),
            m_descriptorSetManager(new DescriptorSetManager()),
            m_resourceManager(new ResourceManager()),
            m_sceneManager(new SceneManager()),
            m_renderManager(new RenderManager()) {
        ASSERT(m_graphicsContext != nullptr);
        ASSERT(m_memoryManager != nullptr);
        ASSERT(m_dataManager != nullptr);
        ASSERT(m_descriptorSetManager != nullptr);
        ASSERT(m_resourceManager != nullptr);
        ASSERT(m_sceneManager != nullptr);
        ASSERT(m_renderManager != nullptr);
        m_instance = this;
        std::cout << "Context created" << std::endl;
    }

    void JoyContext::Init() {
        m_memoryManager->Init();
        m_renderManager->Init();
        m_sceneManager->Init();
    }

    void JoyContext::Start() {
        m_memoryManager->Start();
        m_renderManager->Start();
    }

    void JoyContext::Update() {
        m_sceneManager->Update();
        m_renderManager->Update();
    }

    void JoyContext::Stop() {
        m_renderManager->Stop(); // will destroy managers in reverse order
        m_memoryManager->Stop();
    }

    JoyContext::~JoyContext() {
        Stop();
        delete m_sceneManager; // unregister mesh renderers, remove descriptor set, pipelines, pipeline layouts
        delete m_resourceManager; //delete all scene render data (buffers, textures)
        delete m_renderManager; //delete swapchain, synchronisation, framebuffers
        delete m_memoryManager; //free gpu memory
        delete m_graphicsContext; //delete surface, device, instance
        std::cout << "Context destroyed" << std::endl;
    }

    void JoyContext::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        InternalHandleMessage(uMsg, wParam, lParam);
    }

    void JoyContext::InternalHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {

    }

    JoyGraphicsContext *JoyContext::Graphics() noexcept {
        ASSERT(m_instance != nullptr && m_instance->m_graphicsContext != nullptr);
        return m_instance->m_graphicsContext;
    }

    MemoryManager *JoyContext::Memory() noexcept {
        ASSERT(m_instance != nullptr && m_instance->m_memoryManager != nullptr);
        return m_instance->m_memoryManager;
    }

    DataManager *JoyContext::Data() noexcept {
        ASSERT(m_instance != nullptr && m_instance->m_dataManager != nullptr);
        return m_instance->m_dataManager;
    }

    DescriptorSetManager *JoyContext::DescriptorSet() noexcept {
        ASSERT(m_instance != nullptr && m_instance->m_descriptorSetManager != nullptr);
        return m_instance->m_descriptorSetManager;
    }

    ResourceManager *JoyContext::Resource() noexcept {
        ASSERT(m_instance != nullptr && m_instance->m_resourceManager != nullptr);
        return m_instance->m_resourceManager;
    }

    SceneManager *JoyContext::Scene() noexcept {
        ASSERT(m_instance != nullptr && m_instance->m_sceneManager != nullptr);
        return m_instance->m_sceneManager;
    }

    RenderManager *JoyContext::Render() noexcept {
        ASSERT(m_instance != nullptr && m_instance->m_renderManager != nullptr);
        return m_instance->m_renderManager;
    }
}