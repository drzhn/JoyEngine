#ifndef DESCRIPTOR_SET_MANAGER_H
#define DESCRIPTOR_SET_MANAGER_H

#include "IJoyGraphicsContext.h"
#include "Utils/Assert.h"

namespace JoyEngine {
    class DescriptorSetManager {
    public:

        DescriptorSetManager() = delete;

        explicit DescriptorSetManager(IJoyGraphicsContext *);

        static DescriptorSetManager *GetInstance() noexcept {
            ASSERT(m_instance != nullptr);
            return m_instance;
        }
    private:
        static DescriptorSetManager *m_instance;

        IJoyGraphicsContext *const m_graphicsContext;
        const VkAllocationCallbacks *m_allocator;
    };
}

#endif //DESCRIPTOR_SET_MANAGER_H
