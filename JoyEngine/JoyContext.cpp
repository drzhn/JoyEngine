#include "JoyContext.h"

namespace JoyEngine
{
	GraphicsManager* JoyContext::Graphics = nullptr;

	MemoryManager* JoyContext::Memory = nullptr;

	DataManager* JoyContext::Data = nullptr;

	DescriptorSetManager* JoyContext::DescriptorSet = nullptr;

	ResourceManager* JoyContext::Resource = nullptr;

	SceneManager* JoyContext::Scene = nullptr;

	RenderManager* JoyContext::Render = nullptr;

	void JoyContext::Init(GraphicsManager* graphicsContext, MemoryManager* memoryManager, DataManager* dataManager,
		DescriptorSetManager* descriptorSetManager, ResourceManager* resourceManager, SceneManager* sceneManager,
		RenderManager* renderManager)
	{
		Graphics = graphicsContext;
		Memory = memoryManager;
		Data = dataManager;
		DescriptorSet = descriptorSetManager;
		Resource = resourceManager;
		Scene = sceneManager;
		Render = renderManager;
		
	}
}
