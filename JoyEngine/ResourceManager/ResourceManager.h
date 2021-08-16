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
        const ResourceManager *instance;

        ResourceManager() = default;

        ResourceManager(const MemoryManager &memoryManager, const IJoyGraphicsContext &graphicsContext) :
                m_memoryManager(memoryManager),
                m_graphicsContext(graphicsContext),
                instance(this) {}

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

        template<class T>
        void UnloadResource(GUID guid) { assert(false); }

        template<>
        void UnloadResource<Mesh>(GUID guid);

        template<>
        void UnloadResource<Texture>(GUID guid);

        template<>
        void UnloadResource<Shader>(GUID guid);

    private:
        const MemoryManager &m_memoryManager;
        const IJoyGraphicsContext &m_graphicsContext;

        std::map<GUID, GFXMesh *> m_loadedMeshes;
        std::map<GUID, GFXTexture *> m_loadedTextures;
        std::map<GUID, GFXShader *> m_loadedShaders;

        template<typename T>
        void CreateGPUBuffer(T *data, size_t size,
                             VkBuffer &vertexBuffer,
                             VkDeviceMemory &vertexBufferMemory,
                             VkBufferUsageFlagBits usageFlag);

        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);

        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void DestroyBuffer(VkBuffer vertexBuffer,
                           VkDeviceMemory vertexBufferMemory);

        VkCommandBuffer BeginSingleTimeCommands();

        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

        void CreateTextureImage(const std::string &filename, VkImage &textureImage, VkDeviceMemory &textureImageMemory);

        void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView &imageView);

        void
        CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

        void CreateTextureSampler(VkSampler &textureSampler);

        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        void DestroyImage(VkSampler sampler, VkImageView imageView, VkImage image, VkDeviceMemory imageMemory);

        VkShaderModule CreateShaderModule(const std::string &filename);

        void CreateShaderModule(const std::string &filename, VkShaderModule &shaderModule);

        void DestroyShaderModule(VkShaderModule shaderModule);
    };
}

#endif //RESOURCE_MANAGER_H