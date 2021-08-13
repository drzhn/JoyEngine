#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <set>

#include "MemoryManager/MemoryManager.h"
#include "SceneManager/SceneManager.h"
#include "SceneManager/GameObject.h"
#include "Components/MeshRendererTypes.h"
#include "RenderManager/RenderManager.h"
#include "RenderManager/RenderObject.h"
#include "Utils/GUID.h"
#include "GFXResource.h"

namespace JoyEngine {

    class ResourceManager {
    public:
        ResourceManager() = default;

        ResourceManager(const MemoryManager &memoryManager, const IJoyGraphicsContext &graphicsContext) :
                m_memoryManager(memoryManager),
                m_graphicsContext(graphicsContext) {}

        void Init() {}

        void Start() {}

        void Stop() {}

        template<class T>
        void LoadResource(GUID guid, const std::string &filename) { assert(false); }

        template<>
        void LoadResource<Mesh>(GUID guid, const std::string &filename);

        template<>
        void LoadResource<Texture>(GUID guid, const std::string &filename);

        template<>
        void LoadResource<Shader>(GUID guid, const std::string &filename);

        template<typename T>
        void CreateGPUBuffer(T *data, size_t size,
                                              VkBuffer &vertexBuffer,
                                              VkDeviceMemory &vertexBufferMemory,
                                              VkBufferUsageFlagBits usageFlag);

        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    private:
        const MemoryManager &m_memoryManager;
        const IJoyGraphicsContext &m_graphicsContext;

        std::map<GUID, GFXMesh *> m_loadedMeshes;
        std::map<GUID, GFXTexture *> m_loadedTextures;
        std::map<GUID, GFXShader *> m_loadedShaders;
    };
}

#endif //RESOURCE_MANAGER_H