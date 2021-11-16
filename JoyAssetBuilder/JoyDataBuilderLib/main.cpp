#include <filesystem>
#include <iostream>
#include <vector>

#include "ModelLoader.h"
#include "TextureLoader.h"

//int main()
//{
//	for (uint32_t i = 0; i < 2; i++)
//	{
//		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(dataPaths[i]))
//		{
//			if (!dirEntry.is_regular_file()) continue;
//			std::string extenstion = std::filesystem::path(dirEntry).extension().string();
//			if (extenstion == ".obj")
//			{
//				std::cout << dirEntry << std::endl;
//
//				std::string filename = dirEntry.path().string();
//
//				ModelData model = {};
//				ModelLoader::LoadModel(model.vertices, model.indices,
//				                       model.verticesSize, model.indicesSize,
//				                       filename);
//
//				std::ofstream output(filename + ".data", std::ios::binary);
//				output.write(reinterpret_cast<const char*>(&model.verticesSize), sizeof(uint32_t));
//				output.write(reinterpret_cast<const char*>(&model.indicesSize), sizeof(uint32_t));
//				output.write(reinterpret_cast<const char*>(model.vertices.data()), model.verticesSize);
//				output.write(reinterpret_cast<const char*>(model.indices.data()), model.indicesSize);
//				output.close();
//			}
//			if (extenstion == ".jpeg" || extenstion == ".png" || extenstion == ".bmp" || extenstion == ".jpg")
//			{
//				std::cout << dirEntry << std::endl;
//
//				std::string filename = dirEntry.path().string();
//
//				TextureData texture = {};
//				TexturLoader::LoadTexture(filename, texture.data, texture.width, texture.height);
//
//				std::ofstream output(filename + ".data", std::ios::binary);
//				output.write(reinterpret_cast<const char*>(&texture.width), sizeof(uint32_t));
//				output.write(reinterpret_cast<const char*>(&texture.height), sizeof(uint32_t));
//				output.write(reinterpret_cast<const char*>(texture.data), texture.width * texture.height * STBI_rgb_alpha);
//				output.close();
//			}
//		}
//	}
//	return 0;
//}
std::string errorMessage;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;
std::vector<unsigned char> textureData;

extern "C" __declspec(dllexport) int __cdecl BuildModel(
	const char* modelFileName,
	const void** vertexDataPtr,
	unsigned long long* vertexDataSize,
	const void** indexDataPtr,
	unsigned long long* indexDataSize,
	const char** errorMessageCStr)
{
	const std::string filename = std::string(modelFileName);
	bool res = ModelLoader::LoadModel(vertices, indices, filename, errorMessage);
	if (!res)
	{
		*errorMessageCStr = errorMessage.c_str();
		return 1;
	}

	*vertexDataPtr = vertices.data();
	*vertexDataSize = vertices.size() * sizeof(Vertex);
	*indexDataPtr = indices.data();
	*indexDataSize = indices.size() * sizeof(uint32_t);
	return 0;
}

extern "C" __declspec(dllexport) int __cdecl BuildTexture(
	const char* textureFileName,
	const void** textureDataPtr,
	unsigned long long* textureDataSize,
	uint32_t* textureWidth,
	uint32_t* textureHeight,
	const char** errorMessageCStr)
{
	const std::string filename = std::string(textureFileName);
	bool res = TextureLoader::LoadTexture(filename, textureData, textureWidth, textureHeight);
	if (!res)
	{
		*errorMessageCStr = errorMessage.c_str();
		return 1;
	}
	*textureDataPtr = textureData.data();
	*textureDataSize = textureData.size();

	return 0;
}
