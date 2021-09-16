#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vulkan/vulkan.h>

namespace JoyEngine {

    class JoyContext;

    class MemoryManager {
    public:
        MemoryManager() = default;

        void Init() {}

        void Start() {}

        void Stop() {}

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

        void CreateGPUBuffer(void *data,
                             size_t stride,
                             size_t size,
                             VkBuffer &vertexBuffer,
                             VkDeviceMemory &vertexBufferMemory,
                             VkBufferUsageFlagBits usageFlag);

        void DestroyBuffer(VkBuffer vertexBuffer,
                           VkDeviceMemory vertexBufferMemory);

        void CreateTexture(VkImage &image,
                           VkImageView &imageView,
                           VkDeviceMemory &memory,
                           const unsigned char *data,
                           int length);

        void CreateTextureSampler(VkSampler &textureSampler);

        void DestroyImage(VkImageView imageView, VkImage image, VkDeviceMemory imageMemory);

        void DestroySampler(VkSampler sampler);

        void CreateShaderModule(const uint32_t *code, size_t codeSize, VkShaderModule &shaderModule);

        void DestroyShaderModule(VkShaderModule shaderModule);

    private:
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        VkCommandBuffer BeginSingleTimeCommands();

        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

        void CreateTextureImage(const unsigned char *, int len, VkImage &textureImage, VkDeviceMemory &textureImageMemory);

        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    };
}

#endif