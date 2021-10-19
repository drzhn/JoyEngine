#ifndef MESH_H
#define MESH_H

#include <memory>
#include <fstream>

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

		[[nodiscard]] bool IsLoaded() const noexcept override { return m_vertexBuffer->IsLoaded() && m_indexBuffer->IsLoaded(); }


	private:
		size_t m_indexSize;
		size_t m_vertexSize;

		std::unique_ptr<Buffer> m_vertexBuffer;
		std::unique_ptr<Buffer> m_indexBuffer;

		std::ifstream m_modelStream;
	};
}


#endif //MESH_H
