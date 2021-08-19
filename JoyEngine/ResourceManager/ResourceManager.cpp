#include "ResourceManager.h"

#include <string>
#include <iostream>



#include "Utils/ModelLoader.h"
//#include "Components/MeshRendererTypes.h"
//
#include "Utils/FileUtils.h"
#include "RenderManager/VulkanUtils.h"

namespace JoyEngine {

    ResourceManager *ResourceManager::m_instance = nullptr;

    ResourceManager::ResourceManager(IJoyGraphicsContext *const graphicsContext) :
            m_graphicsContext(graphicsContext),
            m_allocator(graphicsContext->GetAllocationCallbacks()) {
        ResourceManager::m_instance = this;
    }
}