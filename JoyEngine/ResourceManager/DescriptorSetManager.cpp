#include "DescriptorSetManager.h"

namespace JoyEngine {
    DescriptorSetManager *DescriptorSetManager::m_instance = nullptr;

    DescriptorSetManager::DescriptorSetManager(IJoyGraphicsContext *const graphicsContext) :
            m_graphicsContext(graphicsContext),
            m_allocator(graphicsContext->GetAllocationCallbacks()) {
        DescriptorSetManager::m_instance = this;
    }
}