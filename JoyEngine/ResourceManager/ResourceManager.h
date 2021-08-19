#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <set>
#include <cassert>
#include <memory>
#include <vulkan/vulkan.h>

#include "IJoyGraphicsContext.h"

#include "MemoryManager/MemoryManager.h"
//#include "SceneManager/SceneManager.h"
//#include "SceneManager/GameObject.h"
#include "Components/MeshRendererTypes.h"
#include "GFXResource.h"
//#include "RenderManager/RenderManager.h"
//#include "RenderManager/RenderObject.h"
//#include "Utils/GUID.h"
//#include "GFXResource.h"
#include "Utils/ModelLoader.h"
#include "RenderManager/VulkanAllocator.h"
//#include "JoyEngineContext.h"

namespace JoyEngine {

    class IJoyGraphicsContext;

    class JoyGraphicsContext;

    class ResourceManager {
    public:

        ResourceManager() = default;

        ResourceManager(MemoryManager *const, IJoyGraphicsContext *const);

        static ResourceManager *GetInstance() noexcept { return m_instance; }

        void Init() {}

        void Start() {}

        void Stop() {}

        template<class T>
        void LoadResource(GUID guid, const std::string &filename) { assert(false); }

        // implementation is here because of https://stackoverflow.com/questions/456713/why-do-i-get-unresolved-external-symbol-errors-when-using-templates
        template<>
        void LoadResource<Mesh>(GUID guid, const std::string &filename) {
            if (m_loadedMeshes.find(guid) != m_loadedMeshes.end()) {
                m_loadedMeshes[guid]->refCount++;
                return;
            }
            m_loadedMeshes.insert({guid,std::make_unique<GFXMesh>()});
            auto& mesh = m_loadedMeshes[guid];
            mesh->refCount = 1;
            ModelLoader::LoadModel(mesh->vertices, mesh->indices, filename.c_str());
            CreateGPUBuffer(mesh->vertices.data(), sizeof(Vertex), mesh->vertices.size(), mesh->vertexBuffer, mesh->vertexBufferMemory,
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            CreateGPUBuffer(mesh->indices.data(), sizeof(uint32_t), mesh->indices.size(), mesh->indexBuffer, mesh->indexBufferMemory,
                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        }

        template<>
        void LoadResource<Texture>(GUID guid, const std::string &filename) {
            if (m_loadedTextures.find(guid) != m_loadedTextures.end()) {
                m_loadedTextures[guid]->refCount++;
                return;
            }
            m_loadedTextures.insert({guid,std::make_unique<GFXTexture>()});
            auto& texture = m_loadedTextures[guid];
            texture->refCount = 1;
            CreateTexture(texture.get(), filename);
        }

        template<>
        void LoadResource<Shader>(GUID guid, const std::string &filename) {
            if (m_loadedShaders.find(guid) != m_loadedShaders.end()) {
                m_loadedShaders[guid]->refCount++;
                return;
            }
            m_loadedShaders.insert({guid,std::make_unique<GFXShader>()});
            auto& shader = m_loadedShaders[guid];
            shader->refCount = 1;
            CreateShaderModule(filename, shader->shaderModule);
        }

        template<class T>
        void UnloadResource(GUID guid) { assert(false); }

        template<>
        void UnloadResource<Texture>(GUID guid) {
            if (m_loadedTextures.find(guid) != m_loadedTextures.end()) {
                m_loadedTextures[guid]->refCount--;
            } else {
                assert(false);
            }
            if (m_loadedTextures[guid]->refCount == 0) {
                DestroyImage(m_loadedTextures[guid]->textureSampler,
                             m_loadedTextures[guid]->textureImageView,
                             m_loadedTextures[guid]->textureImage,
                             m_loadedTextures[guid]->textureImageMemory);
                m_loadedTextures.erase(guid);
            }
        }

        template<>
        void UnloadResource<Mesh>(GUID guid) {
            if (m_loadedMeshes.find(guid) != m_loadedMeshes.end()) {
                m_loadedMeshes[guid]->refCount--;
            }
            if (m_loadedMeshes[guid]->refCount == 0) {
                DestroyBuffer(m_loadedMeshes[guid]->vertexBuffer, m_loadedMeshes[guid]->vertexBufferMemory);
                DestroyBuffer(m_loadedMeshes[guid]->indexBuffer, m_loadedMeshes[guid]->indexBufferMemory);
                m_loadedMeshes.erase(guid);
            }
        }

        template<>
        void UnloadResource<Shader>(GUID guid) {
            if (m_loadedShaders.find(guid) != m_loadedShaders.end()) {
                m_loadedShaders[guid]->refCount--;
            } else {
                assert(false);
            }
            if (m_loadedShaders[guid]->refCount == 0) {
                DestroyShaderModule(m_loadedShaders[guid]->shaderModule);
                m_loadedShaders.erase(guid);
            }
        }

        void CreateTexture(GFXTexture *texture, const std::string &filename);

        GFXMesh *GetMesh(GUID guid) { return m_loadedMeshes[guid].get(); };

        GFXTexture *GetTexture(GUID guid) { return m_loadedTextures[guid].get(); };

        GFXShader *GetShader(GUID guid) { return m_loadedShaders[guid].get(); };

        static void CreateBuffer(VkPhysicalDevice physicalDevice,
                                 VkDevice logicalDevice,
                                 const VkAllocationCallbacks *allocator,
                                 VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer &buffer,
                                 VkDeviceMemory &bufferMemory);

        static void CreateImage(VkPhysicalDevice physicalDevice,
                                VkDevice logicalDevice,
                                const VkAllocationCallbacks *allocator,
                                uint32_t width,
                                uint32_t height,
                                VkFormat format,
                                VkImageTiling tiling,
                                VkImageUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkImage &image,
                                VkDeviceMemory &imageMemory);

        static void CreateImageView(VkDevice logicalDevice,
                                    const VkAllocationCallbacks *allocator,
                                    VkImage image,
                                    VkFormat format,
                                    VkImageAspectFlags aspectFlags,
                                    VkImageView &imageView);

    private:
        static ResourceManager *m_instance;

        MemoryManager *const m_memoryManager;
        IJoyGraphicsContext *const m_graphicsContext;
        const VkAllocationCallbacks *m_allocator;

        std::map<GUID, std::unique_ptr<GFXMesh>> m_loadedMeshes;
        std::map<GUID, std::unique_ptr<GFXTexture>> m_loadedTextures;
        std::map<GUID, std::unique_ptr<GFXShader>> m_loadedShaders;

        void CreateGPUBuffer(void *data,
                             size_t stride,
                             size_t size,
                             VkBuffer &vertexBuffer,
                             VkDeviceMemory &vertexBufferMemory,
                             VkBufferUsageFlagBits usageFlag);

        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void DestroyBuffer(VkBuffer vertexBuffer,
                           VkDeviceMemory vertexBufferMemory);

        VkCommandBuffer BeginSingleTimeCommands();

        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

        void CreateTextureImage(const std::string &filename, VkImage &textureImage, VkDeviceMemory &textureImageMemory);

        void CreateTextureSampler(VkSampler &textureSampler);

        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        void DestroyImage(VkSampler sampler, VkImageView imageView, VkImage image, VkDeviceMemory imageMemory);

        void CreateShaderModule(const std::string &filename, VkShaderModule &shaderModule);

        void DestroyShaderModule(VkShaderModule shaderModule);
    };
}

#endif //RESOURCE_MANAGER_H