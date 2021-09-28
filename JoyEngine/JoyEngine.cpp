#include "JoyEngine.h"

#include "JoyContext.h"

#include "SceneManager/SceneManager.h"
#include "RenderManager/RenderManager.h"
#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/ResourceManager.h"
#include "DataManager/DataManager.h"
#include "ResourceManager/DescriptorSetManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Common/Time.h"

namespace JoyEngine {

	JoyEngine::JoyEngine(HINSTANCE instance, HWND windowHandle) :
		m_windowHandle(windowHandle),
		m_graphicsContext(new GraphicsManager(instance, windowHandle)),
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

		JoyContext::Init(
			m_graphicsContext,
			m_memoryManager,
			m_dataManager,
			m_descriptorSetManager,
			m_resourceManager,
			m_sceneManager,
			m_renderManager
		);

		std::cout << "Context created" << std::endl;
	}

	void JoyEngine::Init() {
		Time::Init();

		m_memoryManager->Init();
		m_renderManager->Init();
		m_sceneManager->Init();
	}

	void JoyEngine::Start() {
		m_memoryManager->Start();
		m_renderManager->Start();
	}

	void JoyEngine::Update() {
		Time::Update();

		m_sceneManager->Update();
		m_renderManager->Update();
	}

	void JoyEngine::Stop() {
		m_renderManager->Stop(); // will destroy managers in reverse order
		m_memoryManager->Stop();
	}

	JoyEngine::~JoyEngine() {
		Stop();
		delete m_sceneManager; // unregister mesh renderers, remove descriptor set, pipelines, pipeline layouts
		delete m_resourceManager; //delete all scene render data (buffers, textures)
		delete m_renderManager; //delete swapchain, synchronisation, framebuffers
		delete m_memoryManager; //free gpu memory
		delete m_graphicsContext; //delete surface, device, instance
		std::cout << "Context destroyed" << std::endl;
	}

	void JoyEngine::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		InternalHandleMessage(uMsg, wParam, lParam);
	}

	void JoyEngine::InternalHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {

	}
}