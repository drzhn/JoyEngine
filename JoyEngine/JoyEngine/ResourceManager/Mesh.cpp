#include "Mesh.h"

#include "JoyContext.h"

#include <vector>
#include "RenderManager/VulkanTypes.h"
#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine
{
	static uint32_t objCount = 0;
	static uint64_t vertexCount = 0;
	static uint32_t trianglesCount = 0;

	Mesh::Mesh(GUID guid) : Resource(guid)
	{
		std::ifstream modelStream;
		modelStream = JoyContext::Data->GetFileStream(guid, true);

		uint32_t verticesDataSize;
		uint32_t indicesDataSize;
		modelStream.read(reinterpret_cast<char*>(&verticesDataSize), sizeof(uint32_t));
		modelStream.read(reinterpret_cast<char*>(&indicesDataSize), sizeof(uint32_t));

		m_vertexSize = verticesDataSize / sizeof(Vertex);
		m_indexSize = indicesDataSize / sizeof(uint32_t);

		objCount++;
		vertexCount += m_vertexSize;
		trianglesCount += m_indexSize / 3;

		m_vertexBuffer = std::make_unique<Buffer>(
			verticesDataSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_indexBuffer = std::make_unique<Buffer>(
			indicesDataSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_vertexBuffer->LoadDataAsync(modelStream, sizeof(uint32_t) + sizeof(uint32_t));
		m_indexBuffer->LoadDataAsync(modelStream, sizeof(uint32_t) + sizeof(uint32_t) + verticesDataSize);

		if (modelStream.is_open())
		{
			modelStream.close();
		}
		std::string s = std::to_string(objCount) + " " + std::to_string(vertexCount) + " " +
			std::to_string(trianglesCount) + "\n";
		OutputDebugStringA(s.c_str());
	}

	Mesh::~Mesh()
	{
		//JoyContext::Memory->DestroyBuffer(m_vertexBuffer, m_vertexBufferMemory);
		//JoyContext::Memory->DestroyBuffer(m_indexBuffer, m_indexBufferMemory);
	}
}
