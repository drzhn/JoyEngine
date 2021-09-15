#include "ResourceManager.h"

#include <string>
#include <iostream>

#include "JoyContext.h"

#include "Utils/FileUtils.h"
#include "RenderManager/VulkanUtils.h"

namespace JoyEngine {

    ResourceManager::ResourceManager() :
            m_graphicsContext(JoyContext::Graphics()),
            m_allocator(JoyContext::Graphics()->GetAllocationCallbacks()) {
    }
}