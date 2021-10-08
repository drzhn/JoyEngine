#ifndef RESOURCE_H
#define RESOURCE_H

#include "Utils/GUID.h"

namespace JoyEngine {
    class Resource {
    public:
        Resource() = default;

        Resource(GUID guid) :m_guid(guid) {};

        virtual ~Resource() = default;

        [[nodiscard]] uint32_t GetRefCount() const { return m_refCount; }

        void IncreaseRefCount() { m_refCount++; }

        void DecreaseRefCount() { m_refCount--; }

        [[nodiscard]] bool IsLoaded() const noexcept { return m_isLoaded; }
        [[nodiscard]] GUID GetGuid() const noexcept { return m_guid; }

    private:
        uint32_t m_refCount = 0;
        GUID m_guid;

    protected:
        bool m_isLoaded = false;
    };
}

#endif //RESOURCE_H
