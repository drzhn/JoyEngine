#ifndef JOY_CONTEXT_H
#define JOY_CONTEXT_H

namespace JoyEngine
{
	class GraphicsManager;
	class MemoryManager;
	class DataManager;
	class DescriptorSetManager;
	class ResourceManager;
	class SceneManager;
	class RenderManager;

	class JoyContext
	{
	public:
		static void Init(
			GraphicsManager* graphicsContext,
			MemoryManager* memoryManager,
			DataManager* dataManager,
			DescriptorSetManager* descriptorSetManager,
			ResourceManager* resourceManager,
			SceneManager* sceneManager,
			RenderManager* renderManager
		);

		static GraphicsManager* Graphics;
		static MemoryManager* Memory;
		static DataManager* Data;
		static DescriptorSetManager* DescriptorSet;
		static ResourceManager* Resource;
		static SceneManager* Scene;
		static RenderManager* Render;
	};
}

#endif //JOY_CONTEXT_H
