#ifndef DESCRIPTOR_SET_MANAGER_H
#define DESCRIPTOR_SET_MANAGER_H

#include <vector>
#include <map>
#include <list>
#include <memory>

#include "IJoyGraphicsContext.h"
#include "Utils/Assert.h"
#include "Common/Resource.h"

#include <vulkan/vulkan.h>

// rule 1: descriptor pool per descriptor set layout
// rule 2: descriptor set per material which uses this layout
// exact number of set which will be allocated form pool is unknown
// we can count it parsing all materials in the scene during build time
// but for now we set some default number of sets
#define DESCRIPTOR_POOL_SIZE 8

namespace JoyEngine {

    class DescriptorPool {
    public :
        DescriptorPool() = delete;

        DescriptorPool(VkDescriptorSetLayout setLayout, const std::vector<VkDescriptorType> &types);

        ~DescriptorPool();

        VkDescriptorSet Allocate();

        void Free(VkDescriptorSet set);

        [[nodiscard]] uint32_t GetSize() const noexcept;

    private:
        VkDescriptorPool m_pool;
        std::list<VkDescriptorSet> m_freeList;
    };

    class DescriptorPoolList final : public Resource {
    public:
        DescriptorPoolList(VkDescriptorSetLayout setLayout, const std::vector<VkDescriptorType> &types);

        std::vector<VkDescriptorSet> Allocate(uint32_t count);

        void Free(const std::vector<VkDescriptorSet>& descriptorSets);

    private:
        VkDescriptorSetLayout m_setLayout;
        const std::vector<VkDescriptorType> &m_types;

        std::list<std::unique_ptr<DescriptorPool>> m_poolList;
        std::map<VkDescriptorSet, DescriptorPool*> m_usedDescriptorSets;
    };

    class DescriptorSetManager {
    public:

        DescriptorSetManager() = delete;

        explicit DescriptorSetManager(IJoyGraphicsContext *);

        static DescriptorSetManager *GetInstance() noexcept {
            ASSERT(m_instance != nullptr);
            return m_instance;
        }

        void RegisterPool(uint64_t hash, VkDescriptorSetLayout setLayout, const std::vector<VkDescriptorType> &types);

        void UnregisterPool(uint64_t hash);

    private:
        static DescriptorSetManager *m_instance;

        IJoyGraphicsContext *const m_graphicsContext;
        const VkAllocationCallbacks *m_allocator;

        std::map<uint64_t, std::unique_ptr<DescriptorPoolList>> m_pools;
    };
}

#endif //DESCRIPTOR_SET_MANAGER_H
