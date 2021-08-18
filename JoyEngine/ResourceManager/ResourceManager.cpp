#include "ResourceManager.h"

#include <string>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include "Utils/ModelLoader.h"
//#include "Components/MeshRendererTypes.h"
//
#include "Utils/FileUtils.h"
#include "RenderManager/VulkanUtils.h"

namespace JoyEngine {

    ResourceManager *ResourceManager::m_instance = nullptr;

    ResourceManager::ResourceManager(MemoryManager *const memoryManager, IJoyGraphicsContext *const graphicsContext) :
            m_memoryManager(memoryManager),
            m_graphicsContext(graphicsContext) {
        ResourceManager::m_instance = this;
    }

    void ResourceManager::CreateTextureImage(const std::string &filename, VkImage &textureImage, VkDeviceMemory &textureImageMemory) {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(m_graphicsContext->GetVkPhysicalDevice(),
                     m_graphicsContext->GetVkDevice(),
                     m_graphicsContext->GetAllocator(),
                     imageSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                     | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingBufferMemory);

        void *data;
        vkMapMemory(m_graphicsContext->GetVkDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_graphicsContext->GetVkDevice(), stagingBufferMemory);

        stbi_image_free(pixels);

        CreateImage(m_graphicsContext->GetVkPhysicalDevice(),
                    m_graphicsContext->GetVkDevice(),
                    m_graphicsContext->GetAllocator(),
                    texWidth, texHeight,
                    VK_FORMAT_R8G8B8A8_SRGB,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT
                    | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    textureImage, textureImageMemory);

        TransitionImageLayout(textureImage,
                              VK_FORMAT_R8G8B8A8_SRGB,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );
        CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        TransitionImageLayout(textureImage,
                              VK_FORMAT_R8G8B8A8_SRGB,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        vkDestroyBuffer(m_graphicsContext->GetVkDevice(), stagingBuffer, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
        vkFreeMemory(m_graphicsContext->GetVkDevice(), stagingBufferMemory, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
    }

    void ResourceManager::CreateImageView(VkDevice logicalDevice,
                                          Allocator *allocator,
                                          VkImage image,
                                          VkFormat format,
                                          VkImageAspectFlags aspectFlags,
                                          VkImageView &imageView) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(logicalDevice, &viewInfo, allocator->GetAllocationCallbacks(), &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void ResourceManager::CreateImage(VkPhysicalDevice physicalDevice,
                                      VkDevice logicalDevice,
                                      Allocator *allocator,
                                      uint32_t width,
                                      uint32_t height,
                                      VkFormat format,
                                      VkImageTiling tiling,
                                      VkImageUsageFlags usage,
                                      VkMemoryPropertyFlags properties,
                                      VkImage &image,
                                      VkDeviceMemory &imageMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(logicalDevice, &imageInfo, allocator->GetAllocationCallbacks(), &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        std::cout << "Allocation GPU Memory:";
        if (vkAllocateMemory(logicalDevice, &allocInfo, allocator->GetAllocationCallbacks(), &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }
        std::cout << "GPU Allocated:";

        vkBindImageMemory(logicalDevice, image, imageMemory, 0);
    }

    void ResourceManager::CreateTextureSampler(VkSampler &textureSampler) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 1.0f;
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_graphicsContext->GetVkPhysicalDevice(), &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(m_graphicsContext->GetVkDevice(), &samplerInfo, m_graphicsContext->GetAllocator()->GetAllocationCallbacks(), &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void ResourceManager::CreateBuffer(
            VkPhysicalDevice physicalDevice,
            VkDevice logicalDevice,
            Allocator *allocator,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer &buffer,
            VkDeviceMemory &bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(logicalDevice, &bufferInfo, allocator->GetAllocationCallbacks(), &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(logicalDevice, &allocInfo, allocator->GetAllocationCallbacks(), &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }
        vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
    }

    void ResourceManager::CreateShaderModule(const std::string &filename, VkShaderModule &shaderModule) {
        const std::vector<char> code = readFile(filename);
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
        if (vkCreateShaderModule(m_graphicsContext->GetVkDevice(),
                                 &createInfo,
                                 m_graphicsContext->GetAllocator()->GetAllocationCallbacks(), &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
    }

    void ResourceManager::DestroyBuffer(VkBuffer buffer, VkDeviceMemory memory) {
        vkDestroyBuffer(m_graphicsContext->GetVkDevice(), buffer, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
        vkFreeMemory(m_graphicsContext->GetVkDevice(), memory, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
    }

    void ResourceManager::DestroyImage(VkSampler sampler, VkImageView imageView, VkImage image, VkDeviceMemory imageMemory) {
        vkDestroySampler(m_graphicsContext->GetVkDevice(), sampler, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
        vkDestroyImageView(m_graphicsContext->GetVkDevice(), imageView, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
        vkDestroyImage(m_graphicsContext->GetVkDevice(), image, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
        vkFreeMemory(m_graphicsContext->GetVkDevice(), imageMemory, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
    }

    void ResourceManager::DestroyShaderModule(VkShaderModule shaderModule) {
        vkDestroyShaderModule(m_graphicsContext->GetVkDevice(), shaderModule, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
    }

    void ResourceManager::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        EndSingleTimeCommands(commandBuffer);
    }

    void ResourceManager::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                width,
                height,
                1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        EndSingleTimeCommands(commandBuffer);
    }

    void ResourceManager::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );

        EndSingleTimeCommands(commandBuffer);
    }

    VkCommandBuffer ResourceManager::BeginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_graphicsContext->GetVkCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_graphicsContext->GetVkDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void ResourceManager::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_graphicsContext->GetGraphicsVkQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_graphicsContext->GetGraphicsVkQueue());

        vkFreeCommandBuffers(m_graphicsContext->GetVkDevice(), m_graphicsContext->GetVkCommandPool(), 1, &commandBuffer);
    }

    void ResourceManager::CreateGPUBuffer(void *data,
                                          size_t stride,
                                          size_t size,
                                          VkBuffer &vertexBuffer,
                                          VkDeviceMemory &vertexBufferMemory,
                                          VkBufferUsageFlagBits usageFlag) {
        VkDeviceSize bufferSize = stride * size;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(m_graphicsContext->GetVkPhysicalDevice(),
                     m_graphicsContext->GetVkDevice(),
                     m_graphicsContext->GetAllocator(),
                     bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer,
                     stagingBufferMemory);

        void *mappedData;
        vkMapMemory(m_graphicsContext->GetVkDevice(), stagingBufferMemory, 0, bufferSize, 0, &mappedData);
        memcpy(mappedData, data, (size_t) bufferSize);
        vkUnmapMemory(m_graphicsContext->GetVkDevice(), stagingBufferMemory);

        CreateBuffer(m_graphicsContext->GetVkPhysicalDevice(),
                     m_graphicsContext->GetVkDevice(),
                     m_graphicsContext->GetAllocator(),
                     bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     vertexBuffer,
                     vertexBufferMemory);

        CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(m_graphicsContext->GetVkDevice(), stagingBuffer, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
        vkFreeMemory(m_graphicsContext->GetVkDevice(), stagingBufferMemory, m_graphicsContext->GetAllocator()->GetAllocationCallbacks());
    }

    void ResourceManager::CreateTexture(GFXTexture *texture, const std::string &filename) {
        CreateTextureImage(filename, texture->textureImage, texture->textureImageMemory);
        CreateImageView(m_graphicsContext->GetVkDevice(),
                        m_graphicsContext->GetAllocator(),
                        texture->textureImage,
                        VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_ASPECT_COLOR_BIT,
                        texture->textureImageView);
        CreateTextureSampler(texture->textureSampler);
    }
}