#include "ResourceManager.h"

#include "Utils/ModelLoader.h"
#include "RenderManager/VulkanUtils.h"

namespace JoyEngine {
    template<>
    void ResourceManager::LoadResource<Mesh>(GUID guid, const std::string &filename) {
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

    template<typename T>
    void ResourceManager::CreateGPUBuffer(T *data, size_t size,
                                          VkBuffer &vertexBuffer,
                                          VkDeviceMemory &vertexBufferMemory,
                                          VkBufferUsageFlagBits usageFlag) {
        VkDeviceSize bufferSize = sizeof(T) * size;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer,
                     stagingBufferMemory);

        void *mappedData;
        vkMapMemory(m_graphicsContext.GetVkDevice(), stagingBufferMemory, 0, bufferSize, 0, &mappedData);
        memcpy(mappedData, data, (size_t) bufferSize);
        vkUnmapMemory(m_graphicsContext.GetVkDevice(), stagingBufferMemory);

        CreateBuffer(bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageFlag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     vertexBuffer,
                     vertexBufferMemory);

        CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(m_graphicsContext.GetVkDevice(), stagingBuffer, m_graphicsContext.GetAllocator()->GetAllocationCallbacks());
        vkFreeMemory(m_graphicsContext.GetVkDevice(), stagingBufferMemory, m_graphicsContext.GetAllocator()->GetAllocationCallbacks());
    }

    void ResourceManager::CreateBuffer(
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

        if (vkCreateBuffer(m_graphicsContext.GetVkDevice(), &bufferInfo, m_graphicsContext.GetAllocator()->GetAllocationCallbacks(), &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_graphicsContext.GetVkDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(m_graphicsContext.GetVkPhysicalDevice(), memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_graphicsContext.GetVkDevice(), &allocInfo, m_graphicsContext.GetAllocator()->GetAllocationCallbacks(), &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }
    }

    void ResourceManager::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        EndSingleTimeCommands(commandBuffer);
    }

    VkCommandBuffer ResourceManager::BeginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_graphicsContext.GetVkCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_graphicsContext.GetVkDevice(), &allocInfo, &commandBuffer);

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

        vkQueueSubmit(m_graphicsContext.GetGraphicsVkQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_graphicsContext.GetGraphicsVkQueue());

        vkFreeCommandBuffers(m_graphicsContext.GetVkDevice(), m_graphicsContext.GetVkCommandPool(), 1, &commandBuffer);
    }
}