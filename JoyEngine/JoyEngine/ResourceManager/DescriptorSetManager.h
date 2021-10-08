#ifndef DESCRIPTOR_SET_MANAGER_H
#define DESCRIPTOR_SET_MANAGER_H

#include <vector>
#include <map>
#include <list>
#include <memory>

#include "Utils/Assert.h"
#include "Common/Resource.h"

#include <vulkan/vulkan.h>

#include "ResourceManager/Texture.h"

// rule 1: descriptor pool per descriptor set layout
// rule 2: descriptor set per material which uses this layout
// exact number of set which will be allocated form pool is unknown
// we can count it parsing all materials in the scene during build time
// but for now we set some default number of sets
#define DESCRIPTOR_POOL_SIZE 300

namespace JoyEngine
{
	class DescriptorPool
	{
	public :
		DescriptorPool() = delete;

		DescriptorPool(VkDescriptorSetLayout setLayout, std::vector<VkDescriptorType> types);

		~DescriptorPool();

		VkDescriptorSet Allocate();

		void Free(VkDescriptorSet set);

		[[nodiscard]] uint32_t GetSize() const noexcept;

	private:
		VkDescriptorPool m_pool;
		std::list<VkDescriptorSet> m_freeList;
	};

	class DescriptorPoolList
	{
	public:
		DescriptorPoolList(VkDescriptorSetLayout setLayout, std::vector<VkDescriptorType> types);

		std::vector<VkDescriptorSet> Allocate(uint32_t count);

		void Free(VkDescriptorSet descriptorSet);

		[[nodiscard]] uint32_t GetRefCount() const { return m_refCount; }

		void IncreaseRefCount() { m_refCount++; }

		void DecreaseRefCount() { m_refCount--; }

	private:
		uint32_t m_refCount = 0;
		VkDescriptorSetLayout m_setLayout;
		std::vector<VkDescriptorType> m_types;

		std::list<std::unique_ptr<DescriptorPool>> m_poolList;
		std::map<VkDescriptorSet, DescriptorPool*> m_usedDescriptorSets;
	};

	class DescriptorSetManager
	{
	public:
		DescriptorSetManager() = default;

		void Init();

		void RegisterPool(uint64_t hash, VkDescriptorSetLayout setLayout, std::vector<VkDescriptorType> types);

		void UnregisterPool(uint64_t hash);

		std::vector<VkDescriptorSet> Allocate(uint64_t hash, uint32_t count);

		void Free(const std::vector<VkDescriptorSet>& descriptorSets);

		[[nodiscard]] Texture* GetTexture() const;

		~DescriptorSetManager();

	private:
		std::map<uint64_t, std::unique_ptr<DescriptorPoolList>> m_pools;
		std::map<VkDescriptorSet, uint64_t> m_usedDescriptorSets;

		std::unique_ptr<Texture> m_fallbackTexture;
	};
}

#endif //DESCRIPTOR_SET_MANAGER_H
