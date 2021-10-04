#ifndef DESCRIPTOR_BUFFER_H
#define DESCRIPTOR_BUFFER_H

#include <vulkan/vulkan.h>

#include "Common/Resource.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	class DescriptorBuffer final : public Resource
	{
	public:
		DescriptorBuffer() = delete;

		explicit DescriptorBuffer(GUID);

		~DescriptorBuffer() final;

	};
}

#endif
