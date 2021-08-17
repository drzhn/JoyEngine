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
#include "Utils/ModelLoader.h"

namespace JoyEngine {

    class ResourceManager {
    public:

        ResourceManager() = default;

        ResourceManager(const MemoryManager &memoryManager, const IJoyGraphicsContext &graphicsContext);

        static ResourceManager *GetInstance() noexcept { return m_instance; }

        void Init() {}

        void Start() {}

        void Stop() {}

        template<class T>
        void LoadResource(GUID guid, const std::string &filename) { assert(false); }

        template<class T>
        void UnloadResource(GUID guid) { assert(false); }

        // implementation is here because of https://stackoverflow.com/questions/456713/why-do-i-get-unresolved-external-symbol-errors-when-using-templates
        template<>
        void LoadResource<Mesh>(GUID guid, const std::string &filename) {
            if (m_loadedMeshes.find(guid) != m_loadedMeshes.end()) {
                m_loadedMeshes[guid]->refCount++;
                return;
            }
            GFXMesh *mesh = new GFXMesh();
            mesh->refCount = 1;
            ModelLoader::LoadModel(mesh->vertices, mesh->indices, filename.c_str());
            CreateGPUBuffer<Vertex>(mesh->vertices.data(), mesh->vertices.size(), mesh->vertexBuffer, mesh->vertexBufferMemory,
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            CreateGPUBuffer<uint32_t>(mesh->indices.data(), mesh->indices.size(), mesh->indexBuffer, mesh->indexBufferMemory,
                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

            m_loadedMeshes.insert({guid, mesh});
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
        void LoadResource<Texture>(GUID guid, const std::string &filename) {
            if (m_loadedTextures.find(guid) != m_loadedTextures.end()) {
                m_loadedTextures[guid]->refCount++;
                return;
            }
            GFXTexture *texture = new GFXTexture();
            texture->refCount = 1;

            CreateTextureImage(filename, texture->textureImage, texture->textureImageMemory);
            CreateImageView(texture->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, texture->textureImageView);
            CreateTextureSampler(texture->textureSampler);

            m_loadedTextures.insert({guid, texture});
        }

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
        void LoadResource<Shader>(GUID guid, const std::string &filename) {
            if (m_loadedShaders.find(guid) != m_loadedShaders.end()) {
                m_loadedShaders[guid]->refCount++;
                return;
            }
            GFXShader *shader = new GFXShader();
            shader->refCount = 1;
            CreateShaderModule(filename, shader->shaderModule);
            m_loadedShaders.insert({guid, shader});
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

        GFXMesh *GetMesh(GUID guid) { return m_loadedMeshes[guid]; };

        GFXTexture *GetTexture(GUID guid) { return m_loadedTextures[guid]; };

        GFXShader *GetShader(GUID guid) { return m_loadedShaders[guid]; };

        static void CreateBuffer(VkPhysicalDevice physicalDevice,
                                 VkDevice logicalDevice,
                                 Allocator *allocator,
                                 VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer &buffer,
                                 VkDeviceMemory &bufferMemory);

    private:
        static ResourceManager *m_instance;
        const MemoryManager &m_memoryManager;
        const IJoyGraphicsContext &m_graphicsContext;

        std::map<GUID, GFXMesh *> m_loadedMeshes;
        std::map<GUID, GFXTexture *> m_loadedTextures;
        std::map<GUID, GFXShader *> m_loadedShaders;

        template<typename T>
        void CreateGPUBuffer(T *data, size_t size,
                             VkBuffer &vertexBuffer,
                             VkDeviceMemory &vertexBufferMemory,
                             VkBufferUsageFlagBits usageFlag) {
            VkDeviceSize bufferSize = sizeof(T) * size;

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            CreateBuffer(m_graphicsContext.GetVkPhysicalDevice(),
                         m_graphicsContext.GetVkDevice(),
                         m_graphicsContext.GetAllocator(),
                         bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);

            void *mappedData;
            vkMapMemory(m_graphicsContext.GetVkDevice(), stagingBufferMemory, 0, bufferSize, 0, &mappedData);
            memcpy(mappedData, data, (size_t) bufferSize);
            vkUnmapMemory(m_graphicsContext.GetVkDevice(), stagingBufferMemory);

            CreateBuffer(m_graphicsContext.GetVkPhysicalDevice(),
                         m_graphicsContext.GetVkDevice(),
                         m_graphicsContext.GetAllocator(),
                         bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         vertexBuffer,
                         vertexBufferMemory);

            CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

            vkDestroyBuffer(m_graphicsContext.GetVkDevice(), stagingBuffer, m_graphicsContext.GetAllocator()->GetAllocationCallbacks());
            vkFreeMemory(m_graphicsContext.GetVkDevice(), stagingBufferMemory, m_graphicsContext.GetAllocator()->GetAllocationCallbacks());
        }

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

        void CreateShaderModule(const std::string &filename, VkShaderModule &shaderModule);

        void DestroyShaderModule(VkShaderModule shaderModule);
    };
}

#endif //RESOURCE_MANAGER_H