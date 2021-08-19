#ifndef GFX_RESOURCE_H
#define GFX_RESOURCE_H

#include <vulkan/vulkan.h>
#include "Utils/GUID.h"
#include "RenderManager/VulkanTypes.h"

namespace JoyEngine {
    class GFXResource {
    public:
        [[nodiscard]] uint32_t GetRefCount() const { return m_refCount; }

        void IncreaseRefCount() { m_refCount++; }

        void DecreaseRefCount() { m_refCount--; }

    private:
        uint32_t m_refCount = 0;
    };

    class GFXMesh : public GFXResource {
    public :
        GFXMesh() = delete;

        explicit GFXMesh(const std::string &);

        ~GFXMesh();

        [[nodiscard]] size_t GetIndexSize() const noexcept { return m_indexSize; }

        [[nodiscard]] size_t GetVertexSize() const noexcept { return m_vertexSize; }

        [[nodiscard]] VkBuffer GetIndexBuffer() const noexcept { return m_indexBuffer; }

        [[nodiscard]] VkBuffer GetVertexBuffer() const noexcept { return m_vertexBuffer; }

        [[nodiscard]] VkDeviceMemory GetIndexBufferMemory() const noexcept { return m_indexBufferMemory; }

        [[nodiscard]] VkDeviceMemory GetVertexBufferMemory() const noexcept { return m_vertexBufferMemory; }

    private :
        size_t m_indexSize;
        size_t m_vertexSize;

        VkBuffer m_vertexBuffer;
        VkBuffer m_indexBuffer;

        VkDeviceMemory m_vertexBufferMemory;
        VkDeviceMemory m_indexBufferMemory;
    };

    class GFXTexture : public GFXResource {
    public :
        GFXTexture() = default; // just to make creation of empty texture possible (for render manager depth texture)

        explicit GFXTexture(const std::string &);

        ~GFXTexture();

        [[nodiscard]] VkImage &GetImage() noexcept { return m_textureImage; }

        [[nodiscard]] VkDeviceMemory &GetDeviceMemory() noexcept { return m_textureImageMemory; }

        [[nodiscard]] VkImageView &GetImageView() noexcept { return m_textureImageView; }

        [[nodiscard]] VkSampler &GetSampler() noexcept { return m_textureSampler; }

    private :
        VkImage m_textureImage = VK_NULL_HANDLE;
        VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
        VkImageView m_textureImageView = VK_NULL_HANDLE;
        VkSampler m_textureSampler = VK_NULL_HANDLE;
    };

    class GFXShader : public GFXResource {
    public :

        GFXShader() = delete;

        explicit GFXShader(const std::string &);

        ~GFXShader();
        [[nodiscard]] VkShaderModule &GetShadeModule() noexcept { return m_shaderModule; }
    private :
        VkShaderModule m_shaderModule;
    };
}

#endif //GFX_RESOURCE_H
