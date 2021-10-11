#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#define STB_IMAGE_IMPLEMENTATION
#include <string>

#include "stb_image.h"

class TexturLoader
{
public:
	static void LoadTexture(const std::string& filename, unsigned char* & data, uint32_t& width, uint32_t& height)
	{
		int texChannels;
		data = stbi_load(
			filename.c_str(),
			reinterpret_cast<int*>(&width),
			reinterpret_cast<int*>(&height), &texChannels, STBI_rgb_alpha);
	}
};

#endif //TEXTURE_LOADER_H
