#ifndef RESOURCE_H
#define RESOURCE_H

namespace JoyEngine {
    class Resource {
    public:
        Resource() = default;

        virtual ~Resource() = default;

        [[nodiscard]] uint32_t GetRefCount() const { return m_refCount; }

        void IncreaseRefCount() { m_refCount++; }

        void DecreaseRefCount() { m_refCount--; }

    private:
        uint32_t m_refCount = 0;
    };
}

#endif //RESOURCE_H
