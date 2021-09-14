#include "DescriptorSetManager.h"

namespace JoyEngine {
    DescriptorSetManager *DescriptorSetManager::m_instance = nullptr;

    DescriptorSetManager::DescriptorSetManager(IJoyGraphicsContext *const graphicsContext) :
            m_graphicsContext(graphicsContext),
            m_allocator(graphicsContext->GetAllocationCallbacks()) {
        DescriptorSetManager::m_instance = this;
    }

    void DescriptorSetManager::RegisterPool(uint64_t hash, VkDescriptorSetLayout setLayout, const std::vector<VkDescriptorType> &types) {
        if (m_pools.find(hash) != m_pools.end()) {
            m_pools[hash]->IncreaseRefCount();
            return;
        }
        m_pools.insert({hash, std::make_unique<DescriptorPoolList>(setLayout, types)});
        m_pools[hash]->IncreaseRefCount();
    }

    void DescriptorSetManager::UnregisterPool(uint64_t hash) {
        if (m_pools.find(hash) != m_pools.end()) {
            m_pools[hash]->DecreaseRefCount();
        } else {
            ASSERT(false);
        }
        if (m_pools[hash]->GetRefCount() == 0) {
            m_pools.erase(hash);
        }
    }

// =========== Descriptor Pool  =============

    DescriptorPool::DescriptorPool(VkDescriptorSetLayout setLayout, const std::vector<VkDescriptorType> &types) {
        uint32_t poolSize = types.size();
        VkDescriptorPoolSize poolSizes[poolSize];
        for (uint32_t i = 0; i < poolSize; i++) {
            poolSizes[i] = {
                    types[i],
                    DESCRIPTOR_POOL_SIZE
            };
        }

        VkDescriptorPoolCreateInfo poolInfo{
                VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                nullptr,
                0,
                DESCRIPTOR_POOL_SIZE,
                poolSize,
                poolSizes
        };

        VkResult res = vkCreateDescriptorPool(IJoyGraphicsContext::GetInstance()->GetVkDevice(),
                                              &poolInfo,
                                              IJoyGraphicsContext::GetInstance()->GetAllocationCallbacks(),
                                              &m_pool);
        ASSERT(res == VK_SUCCESS);

        VkDescriptorSetLayout layouts[DESCRIPTOR_POOL_SIZE];
        for (int i = 0; i < DESCRIPTOR_POOL_SIZE; i++) layouts[i] = setLayout;

        VkDescriptorSetAllocateInfo allocInfo{
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                nullptr,
                m_pool,
                DESCRIPTOR_POOL_SIZE,
                layouts
        };

        VkDescriptorSet sets[DESCRIPTOR_POOL_SIZE];
        res = vkAllocateDescriptorSets(IJoyGraphicsContext::GetInstance()->GetVkDevice(), &allocInfo, sets);
        ASSERT(res == VK_SUCCESS);
        for (int i = 0; i < DESCRIPTOR_POOL_SIZE; i++) {
            m_freeList.push_back(sets[i]);
        }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(IJoyGraphicsContext::GetInstance()->GetVkDevice(),
                                m_pool,
                                IJoyGraphicsContext::GetInstance()->GetAllocationCallbacks());
    }

    VkDescriptorSet DescriptorPool::Allocate() {
        VkDescriptorSet ret = m_freeList.back();
        m_freeList.pop_back();
        return ret;
    }

    void DescriptorPool::Free(VkDescriptorSet set) {
        m_freeList.push_back(set);
    }

    uint32_t DescriptorPool::GetSize() const noexcept {
        return m_freeList.size();
    }


// =========== Pool List =============

    DescriptorPoolList::DescriptorPoolList(VkDescriptorSetLayout setLayout, const std::vector<VkDescriptorType> &types) :
            m_setLayout(setLayout),
            m_types(types) {
        m_poolList.emplace_back(std::make_unique<DescriptorPool>(setLayout, types));
    }

    std::vector<VkDescriptorSet> DescriptorPoolList::Allocate(uint32_t count) {
        std::vector<VkDescriptorSet> sets;
        uint32_t numAllocated = 0;
        for (auto &pool: m_poolList) {
            while (pool->GetSize() > 0 || numAllocated < count) {
                VkDescriptorSet set = pool->Allocate();
                m_usedDescriptorSets.insert({set, pool.get()});
                sets.push_back(set);
                numAllocated++;
            }
        }
        while (numAllocated < count) {
            std::unique_ptr<DescriptorPool> newPool = std::make_unique<DescriptorPool>(m_setLayout, m_types);

            while (newPool->GetSize() > 0 || numAllocated < count) {
                VkDescriptorSet set = newPool->Allocate();
                m_usedDescriptorSets.insert({set, newPool.get()});
                sets.push_back(set);
                numAllocated++;
            }
            m_poolList.emplace_back(std::move(newPool));
        }
        return sets;
    }

    void DescriptorPoolList::Free(const std::vector<VkDescriptorSet> &descriptorSets) {
        for (const auto &item: descriptorSets) {
            m_usedDescriptorSets[item]->Free(item);
            m_usedDescriptorSets.erase(item);
        }
    }
}