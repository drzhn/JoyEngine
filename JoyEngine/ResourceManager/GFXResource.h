#ifndef GFX_RESOURCE_H
#define GFX_RESOURCE_H

#include <vulkan/vulkan.h>
#include "RenderManager/VulkanTypes.h"

namespace JoyEngine {
    struct GFXResource {
        uint32_t refCount = 0;
    };
    struct GFXMesh : GFXResource {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;

        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
    };

    struct GFXTexture : GFXResource {
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
    };

    struct GFXShader : GFXResource {
        VkShaderModule shaderModule;
    };
}

#endif //GFX_RESOURCE_H
