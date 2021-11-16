#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#define STB_IMAGE_IMPLEMENTATION
#include <string>

#include "stb_image.h"

class TextureLoader
{
public:
	[[nodiscard]]
	static bool LoadTexture(const std::string& filename, std::vector<unsigned char>& data,
	                        uint32_t* width,
	                        uint32_t* height)
	{
		int texChannels;
		unsigned char* dataPtr = stbi_load(
			filename.c_str(),
			reinterpret_cast<int*>(width),
			reinterpret_cast<int*>(height), &texChannels, STBI_rgb_alpha);
		if (dataPtr == nullptr)
		{
			return false;
		}
		size_t dataSize = *width * *height * STBI_rgb_alpha;
		data.resize(dataSize);
		memcpy(data.data(), dataPtr, dataSize);
		delete[] dataPtr;
		return true;
	}
};

#endif //TEXTURE_LOADER_H
