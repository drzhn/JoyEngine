#ifndef JOY_ENGINE_H
#define JOY_ENGINE_H

#include <memory>

#include "windows.h"

namespace JoyEngine
{
	class InputManager;

	class GraphicsManager;

	class MemoryManager;

	class DataManager;

	class DescriptorSetManager;

	class ResourceManager;

	class SceneManager;

	class RenderManager;

	class IWindowMessageHandler
	{
	public:
		virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	};

	class JoyEngine final : public IWindowMessageHandler
	{
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
		HWND m_windowHandle;

		std::unique_ptr<InputManager> m_inputManager;
		std::unique_ptr<GraphicsManager> m_graphicsContext;
		std::unique_ptr<MemoryManager> m_memoryManager;
		std::unique_ptr<DataManager> m_dataManager;
		std::unique_ptr<DescriptorSetManager> m_descriptorSetManager;
		std::unique_ptr<ResourceManager> m_resourceManager;
		std::unique_ptr<SceneManager> m_sceneManager;
		std::unique_ptr<RenderManager> m_renderManager;
	};
}


#endif//JOY_ENGINE_H