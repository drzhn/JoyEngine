#ifndef MESH_H
#define MESH_H

#include <vulkan/vulkan.h>

#include "Common/Resource.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
    class Mesh final: public Resource {
    public :
        Mesh() = delete;

        explicit Mesh(GUID);

        ~Mesh() final;

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
}


#endif //MESH_H
