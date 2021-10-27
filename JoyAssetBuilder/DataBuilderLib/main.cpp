#include <filesystem>
#include <iostream>
#include <vector>

#include "ModelLoader.h"
#include "TextureLoader.h"

struct TextureData
{
	uint32_t width;
	uint32_t height;
	unsigned char* data;
};

struct ModelData
{
	uint32_t verticesSize;
	uint32_t indicesSize;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

const char* dataPaths[] = {"D:/CppProjects/JoyEngine/JoyData/models", "D:/CppProjects/JoyEngine/JoyData/textures"};

int main()
{
	for (uint32_t i = 0; i < 2; i++)
	{
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(dataPaths[i]))
		{
			if (!dirEntry.is_regular_file()) continue;
			std::string extenstion = std::filesystem::path(dirEntry).extension().string();
			if (extenstion == ".obj")
			{
				std::cout << dirEntry << std::endl;

				std::string filename = dirEntry.path().string();

				ModelData model = {};
				ModelLoader::LoadModel(model.vertices, model.indices,
				                       model.verticesSize, model.indicesSize,
				                       filename);

				std::ofstream output(filename + ".data", std::ios::binary);
				output.write(reinterpret_cast<const char*>(&model.verticesSize), sizeof(uint32_t));
				output.write(reinterpret_cast<const char*>(&model.indicesSize), sizeof(uint32_t));
				output.write(reinterpret_cast<const char*>(model.vertices.data()), model.verticesSize);
				output.write(reinterpret_cast<const char*>(model.indices.data()), model.indicesSize);
				output.close();
			}
			if (extenstion == ".jpeg" || extenstion == ".png" || extenstion == ".bmp" || extenstion == ".jpg")
			{
				std::cout << dirEntry << std::endl;

				std::string filename = dirEntry.path().string();

				TextureData texture = {};
				TexturLoader::LoadTexture(filename, texture.data, texture.width, texture.height);

				std::ofstream output(filename + ".data", std::ios::binary);
				output.write(reinterpret_cast<const char*>(&texture.width), sizeof(uint32_t));
				output.write(reinterpret_cast<const char*>(&texture.height), sizeof(uint32_t));
				output.write(reinterpret_cast<const char*>(texture.data), texture.width * texture.height * STBI_rgb_alpha);
				output.close();
			}
		}
	}
	return 0;
}
