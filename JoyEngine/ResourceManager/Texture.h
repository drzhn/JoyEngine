#ifndef TEXTURE_H
#define TEXTURE_H

#include <vulkan/vulkan.h>

#include "Resource.h"
#include "Utils/GUID.h"

namespace JoyEngine {
    class Texture final: public Resource {
    public :
        Texture() = default; // just to make creation of empty texture possible (for render manager depth texture)

        explicit Texture(GUID);

        ~Texture() final;

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
}


#endif //TEXTURE_H
