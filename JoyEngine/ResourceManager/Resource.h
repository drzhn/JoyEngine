#ifndef GFX_RESOURCE_H
#define GFX_RESOURCE_H

#include <vulkan/vulkan.h>
#include "Utils/GUID.h"
#include "RenderManager/VulkanTypes.h"

namespace JoyEngine {

    class Resource {
    public:
        Resource() = default;

        virtual ~Resource() = default;

        [[nodiscard]] uint32_t GetRefCount() const { return m_refCount; }

        void IncreaseRefCount() { m_refCount++; }

        void DecreaseRefCount() { m_refCount--; }

    private:
        uint32_t m_refCount = 0;
    };

    class SharedMaterial : public Resource {
    public :
        SharedMaterial() = delete;

        explicit SharedMaterial(GUID);

        ~SharedMaterial();

    private :
        GUID m_vertexShader;
        GUID m_fragmentShader;
        bool m_hasVertexInput;
        bool m_hasMVP;
        bool m_depthTest;
        bool m_depthWrite;
        std::vector<std::vector<std::tuple<std::string, std::string>>> m_bindingSets;

    };

    class Material : public Resource {
    public :
        Material() = delete;

        explicit Material(GUID);

        ~Material();

    private :
        GUID m_sharedMaterialGuid;
        std::map<std::string, GUID> m_bindings;
    };

    class Mesh : public Resource {
    public :
        Mesh() = delete;

        explicit Mesh(GUID);

        ~Mesh();

        [[nodiscard]] size_t GetIndexSize() const noexcept { return m_indexSize; }

        [[nodiscard]] size_t GetVertexSize() const noexcept { return m_vertexSize; }

        [[nodiscard]] VkBuffer GetIndexBuffer() const noexcept { return m_indexBuffer; }

        [[nodiscard]] VkBuffer GetVertexBuffer() const noexcept { return m_vertexBuffer; }

        [[nodiscard]] VkDeviceMemory GetIndexBufferMemory() const noexcept { return m_indexBufferMemory; }

        [[nodiscard]] VkDeviceMemory GetVertexBufferMemory() const noexcept { return m_vertexBufferMemory; }

    private :
        size_t m_indexSize;
        size_t m_vertexSize;

        VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
        VkBuffer m_indexBuffer = VK_NULL_HANDLE;

        VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
        VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;
    };

    class Texture : public Resource {
    public :
        Texture() = default; // just to make creation of empty texture possible (for render manager depth texture)

        explicit Texture(GUID);

        ~Texture();

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

    class Shader : public Resource {
    public :

        Shader() = delete;

        explicit Shader(GUID);

        ~Shader();

        [[nodiscard]] VkShaderModule &GetShadeModule() noexcept { return m_shaderModule; }

    private :
        VkShaderModule m_shaderModule = VK_NULL_HANDLE;
    };
}

#endif //GFX_RESOURCE_H
