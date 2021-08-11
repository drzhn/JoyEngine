#ifndef VULKANCMAKEPROJECT_ALLOCATOR_H
#define VULKANCMAKEPROJECT_ALLOCATOR_H

#include <iostream>

#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

class Allocator {
private:
    size_t amount = 0;
    const VkAllocationCallbacks result = {
            (void *) this,
            Allocation,
            Reallocation,
            Free,
            nullptr,
            nullptr
    };
public:

// Operator that allows an instance of this class to be used as a
// VkAllocationCallbacks structure
    const inline VkAllocationCallbacks* GetAllocationCallbacks() {
        return &result;
    };

private:
// Declare the allocator callbacks as static member functions.
    static void *VKAPI_CALL Allocation(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
        return static_cast<Allocator *>(pUserData)->Allocation(size, alignment, allocationScope);
    }

    static void *VKAPI_CALL Reallocation(
            void *pUserData,
            void *pOriginal,
            size_t size,
            size_t alignment,
            VkSystemAllocationScope allocationScope) {
        return static_cast<Allocator *>(pUserData)->Reallocation(pOriginal,
                                                                 size,
                                                                 alignment,
                                                                 allocationScope);
    }

    static void VKAPI_CALL Free(
            void *pUserData,
            void *pMemory) {
        return static_cast<Allocator *>(pUserData)->Free(pMemory);
    }

// Now declare the nonstatic member functions that will actually    perform
// the allocations.

    void *Allocation(
            size_t size,
            size_t alignment,
            VkSystemAllocationScope allocationScope) {
        amount += size;
        //std::cout << "Allocation " << size << "b, alignment: " << alignment << "b, total: " << amount << '\n';
        return _aligned_malloc(size, alignment);
    }

    void *Reallocation(
            void *pOriginal,
            size_t size,
            size_t alignment,
            VkSystemAllocationScope allocationScope) {
        //std::cout << "Reallocation " << size << " bytes\n";
        return _aligned_realloc(pOriginal, size, alignment);
    }

    void Free(void *pMemory) {
        //std::cout << "Free \n";

        _aligned_free(pMemory);
    }
};


#endif //VULKANCMAKEPROJECT_ALLOCATOR_H
