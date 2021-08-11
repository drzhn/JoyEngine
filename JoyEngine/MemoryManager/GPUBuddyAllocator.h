#ifndef GPUALLOCATOR_H
#define GPUALLOCATOR_H

#include <cstdint>
#include <cassert>
#include <vector>


namespace JoyEngine {
// Memory allocation for specific memory type
// Note: This class implements only Buddy Memory Allocation
// Note: implemented only for vulkan api

#define GPU_MEMORY_CHUNK_SIZE  65536 // 64 KB
#define GPU_MEMORY_AREA_CHUNK_AMOUNT  256 // 16 MB with 64K chunk size


    enum ChunkState : char {
        available,
        split,
        allocated
    };

    class GPUMemoryArea {
    public:
        GPUMemoryArea() : GPUMemoryArea(GPU_MEMORY_CHUNK_SIZE, GPU_MEMORY_AREA_CHUNK_AMOUNT) {
        }

        GPUMemoryArea(uint32_t chunkSize, uint32_t chunkAmount) :
                m_chunkSize(chunkSize),
                m_chunkAmount(chunkAmount),
                m_maxPower(nearPow(chunkAmount)),
                m_memoryTree(chunkAmount * 2 - 1) {
            for (int i = 0; i < m_chunkAmount * 2 - 1; i++) { m_memoryTree[i] = ChunkState::available; }
        }

        bool Allocate(uint64_t size, uint32_t &offset) {
            assert(size != 0);
            uint32_t numChunks = ((size - 1) / m_chunkSize + 1);
            uint32_t pow = nearPow(numChunks);
            if (FindFreeBlock(m_maxPower, 0, pow, offset)) {
                offset = (offset + 1) * (1 << pow) - (1 << m_maxPower); // find real offset (not in the tree view but in the real memory)
                return true;
            }
            return false;
        }


        void Free(uint64_t size, uint32_t realOffset) {
            assert(size != 0);
            uint32_t numChunks = ((size - 1) / m_chunkSize + 1);
            uint32_t pow = nearPow(numChunks);
            FreeBlock(realOffset, pow);
        }

        inline GPUMemoryArea *GetNext() const noexcept { return m_next; }

    private:
        const uint32_t m_chunkSize;
        const uint32_t m_chunkAmount;
        const uint32_t m_maxPower;
        std::vector<ChunkState> m_memoryTree;
        GPUMemoryArea *m_next = nullptr;

        static uint32_t nearPow(uint32_t num) {
            int pow = 0;
            num -= 1;
            while (num > 0) {
                num >>= 1;
                pow++;
            }
            return pow;
        }

        inline uint32_t getTreeOffset(uint32_t realOffset, uint32_t pow) {
            return (realOffset + (1 << m_maxPower)) / (1 << pow) - 1;
        }

        bool FindFreeBlock(uint32_t currentPow, uint32_t currentIndex, uint32_t pow, uint32_t &foundIndex) {
            if (currentPow == pow) {
                if (m_memoryTree[currentIndex] == available) {
                    m_memoryTree[currentIndex] = allocated;
                    foundIndex = currentIndex;
                    return true;
                }
                return false;
            } else {
                if (m_memoryTree[currentIndex] == allocated) {
                    return false;
                }
                m_memoryTree[currentIndex] = split;
                uint32_t blockStartDelta = currentIndex - ((1 << (m_maxPower - currentPow)) - 1);
                uint32_t childIndex1 = ((1 << (m_maxPower - currentPow + 1)) - 1) + blockStartDelta * 2 + 0;
                uint32_t childIndex2 = ((1 << (m_maxPower - currentPow + 1)) - 1) + blockStartDelta * 2 + 1;
                return FindFreeBlock(currentPow - 1, childIndex1, pow, foundIndex) ||
                       FindFreeBlock(currentPow - 1, childIndex2, pow, foundIndex);
            }
        }

        void FreeBlock(uint32_t realOffset, uint32_t pow) {
            for (uint32_t i = pow; i <= m_maxPower; i++) {
                uint32_t offset = getTreeOffset(realOffset, i);
                m_memoryTree[offset] = available;
                uint32_t neighbourIndex = getTreeOffset(realOffset ^ (1 << i), i);
                if (m_memoryTree[neighbourIndex] == available) {
                } else {
                    break;
                }
            }
        }

    };

    class GPUAllocator {

    };
}
#endif //GPUALLOCATOR_H
