#include "Mesh.h"

#include "JoyContext.h"

#include <vector>
#include "RenderManager/VulkanTypes.h"
#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"
#include "Utils/ModelLoader.h"

namespace JoyEngine
{
	Mesh::Mesh(GUID guid) : Resource(guid)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::ifstream modelStream;
		JoyContext::Data->GetDataStream(modelStream, guid);
		ModelLoader::LoadModel(vertices, indices, modelStream);
		m_vertexSize = vertices.size();
		m_indexSize = indices.size();
		m_vertexBuffer = std::make_unique<Buffer>(
			vertices.size() * sizeof(Vertex),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_indexBuffer = std::make_unique<Buffer>(
			vertices.size() * sizeof(Vertex),
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		JoyContext::Memory->LoadDataToBuffer(
			vertices.data(),
			sizeof(Vertex) * vertices.size(),
			m_vertexBuffer->GetBuffer());

		JoyContext::Memory->LoadDataToBuffer(
			indices.data(),
			sizeof(uint32_t) * indices.size(),
			m_indexBuffer->GetBuffer());

		if (modelStream.is_open())
		{
			modelStream.close();
		}
	}

	Mesh::~Mesh()
	{
		//JoyContext::Memory->DestroyBuffer(m_vertexBuffer, m_vertexBufferMemory);
		//JoyContext::Memory->DestroyBuffer(m_indexBuffer, m_indexBufferMemory);
	}
}
