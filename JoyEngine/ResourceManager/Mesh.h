#ifndef MESH_H
#define MESH_H

#include <memory>

#include <vulkan/vulkan.h>

#include "Common/Resource.h"
#include "ResourceManager/Buffer.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	class Mesh final : public Resource
	{
	public:
		Mesh() = delete;

		explicit Mesh(GUID);

		~Mesh() final;

		[[nodiscard]] size_t GetIndexSize() const noexcept { return m_indexSize; }

		[[nodiscard]] size_t GetVertexSize() const noexcept { return m_vertexSize; }

		[[nodiscard]] VkBuffer GetIndexBuffer() const noexcept { return m_indexBuffer->GetBuffer(); }

		[[nodiscard]] VkBuffer GetVertexBuffer() const noexcept { return m_vertexBuffer->GetBuffer(); }


	private:
		size_t m_indexSize;
		size_t m_vertexSize;

		std::unique_ptr<Buffer> m_vertexBuffer;
		std::unique_ptr<Buffer> m_indexBuffer;
	};
}


#endif //MESH_H
