#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include "tiny_obj_loader.h"
#include <stdexcept>
#include <fstream>

#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

class ModelLoader
{
public:
	static void getFileStream(std::ifstream& stream, const std::string& filename)
	{
		stream.open(filename);
		if (!stream.is_open())
		{
			throw std::runtime_error("Cannot open stream");
		}
	}

	static void LoadModel(std::vector<Vertex>& vertices,
	                      std::vector<uint32_t>& indices,
	                      uint32_t& verticesDataSize,
	                      uint32_t& indicesDataSize,
	                      const std::string& filename)
	{
		std::ifstream stream;
		getFileStream(stream, filename);

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &stream);

		uint32_t vertSize = 0;
		for (const auto& shape : shapes)
		{
			vertSize += shape.mesh.indices.size();
		}
		vertices.resize(vertSize);
		indices.resize(vertSize);

		verticesDataSize = vertSize * sizeof(Vertex);
		indicesDataSize = vertSize * sizeof(uint32_t);

		uint32_t vertIndex = 0;
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				vertices[vertIndex].pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				vertices[vertIndex].normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
				vertices[vertIndex].texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertices[vertIndex].color = {1.0f, 1.0f, 1.0f};

				indices[vertIndex] = vertIndex;
				vertIndex++;
			}
		}
		if (stream.is_open())
		{
			stream.close();
		}
	}
};


#endif //MODEL_LOADER_H
