#include "Mesh.h"

#include "JoyContext.h"

#include <vector>
#include "RenderManager/VulkanTypes.h"
#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine
{
	Mesh::Mesh(GUID guid) : Resource(guid)
	{
		m_modelStream = JoyContext::Data->GetFileStream(guid, true);

		uint32_t verticesDataSize;
		uint32_t indicesDataSize;
		m_modelStream.read(reinterpret_cast<char*>(&verticesDataSize), sizeof(uint32_t));
		m_modelStream.read(reinterpret_cast<char*>(&indicesDataSize), sizeof(uint32_t));

		m_vertexSize = verticesDataSize / sizeof(Vertex);
		m_indexSize = indicesDataSize / sizeof(uint32_t);

		m_vertexBuffer = std::make_unique<Buffer>(
			verticesDataSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_indexBuffer = std::make_unique<Buffer>(
			indicesDataSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_vertexBuffer->LoadDataAsync(m_modelStream, sizeof(uint32_t) + sizeof(uint32_t), [this]()
		{
			if (m_indexBuffer->IsLoaded())
			{
				if (m_modelStream.is_open())
				{
					m_modelStream.close();
				}
			}
		});
		m_indexBuffer->LoadDataAsync(m_modelStream, sizeof(uint32_t) + sizeof(uint32_t) + verticesDataSize, [this]()
		{
			if (m_vertexBuffer->IsLoaded())
			{
				if (m_modelStream.is_open())
				{
					m_modelStream.close();
				}
			}
		});
	}

	Mesh::~Mesh()
	{
		//JoyContext::Memory->DestroyBuffer(m_vertexBuffer, m_vertexBufferMemory);
		//JoyContext::Memory->DestroyBuffer(m_indexBuffer, m_indexBufferMemory);
	}
}
