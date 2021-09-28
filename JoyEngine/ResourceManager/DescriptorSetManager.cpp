#include "DescriptorSetManager.h"

#include "JoyContext.h"

#include "GraphicsManager/GraphicsManager.h"
#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine {

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

    std::vector<VkDescriptorSet> DescriptorSetManager::Allocate(uint64_t hash, uint32_t count) {
        std::vector<VkDescriptorSet> sets = m_pools[hash]->Allocate(count);
        for (const auto &set: sets) {
            m_usedDescriptorSets.insert({set, hash});
        }
        return sets;
    }

    void DescriptorSetManager::Free(const std::vector<VkDescriptorSet> &descriptorSets) {
        for (const auto &descriptorSet: descriptorSets) {
            uint64_t hash = m_usedDescriptorSets[descriptorSet];
            m_pools[hash]->Free(descriptorSet);
            m_usedDescriptorSets.erase(descriptorSet);
        }
    }

// =========== Descriptor Pool  =============

    DescriptorPool::DescriptorPool(VkDescriptorSetLayout setLayout, const std::vector<VkDescriptorType> &types) {
        uint32_t poolSize = types.size();
        std::vector<VkDescriptorPoolSize> poolSizes(poolSize);
        for (uint32_t i = 0; i < poolSize; i++) {
            VkDescriptorPoolSize size = {
                    types[i],
                    DESCRIPTOR_POOL_SIZE
            };
            poolSizes[i] = size;
        }

        VkDescriptorPoolCreateInfo poolInfo{
                VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                nullptr,
                0,
                DESCRIPTOR_POOL_SIZE,
                poolSize,
                poolSizes.data()
        };

        VkResult res = vkCreateDescriptorPool(
                JoyContext::Graphics->GetVkDevice(),
                &poolInfo,
                JoyContext::Graphics->GetAllocationCallbacks(),
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
        res = vkAllocateDescriptorSets(JoyContext::Graphics->GetVkDevice(), &allocInfo, sets);
        ASSERT(res == VK_SUCCESS);
        for (int i = 0; i < DESCRIPTOR_POOL_SIZE; i++) {
            m_freeList.push_back(sets[i]);
        }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(JoyContext::Graphics->GetVkDevice(),
                                m_pool,
                                JoyContext::Graphics->GetAllocationCallbacks());
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
            while (pool->GetSize() > 0 && numAllocated < count) {
                VkDescriptorSet set = pool->Allocate();
                m_usedDescriptorSets.insert({set, pool.get()});
                sets.push_back(set);
                numAllocated++;
            }
        }
        while (numAllocated < count) {
            std::unique_ptr<DescriptorPool> newPool = std::make_unique<DescriptorPool>(m_setLayout, m_types);

            while (newPool->GetSize() > 0 && numAllocated < count) {
                VkDescriptorSet set = newPool->Allocate();
                m_usedDescriptorSets.insert({set, newPool.get()});
                sets.push_back(set);
                numAllocated++;
            }
            m_poolList.emplace_back(std::move(newPool));
        }
        return sets;
    }

    void DescriptorPoolList::Free(VkDescriptorSet descriptorSet) {
        m_usedDescriptorSets[descriptorSet]->Free(descriptorSet);
        m_usedDescriptorSets.erase(descriptorSet);
        // TODO after Free() we need to delete each pool where GetSize() == DESCRIPTOR_POOL_SIZE except one
    }
}