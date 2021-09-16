#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <set>
#include <Utils/Assert.h>
#include <memory>
#include <vulkan/vulkan.h>

#include "JoyGraphicsContext.h"

#include "Common/Resource.h"
#include "Mesh.h"
#include "Material.h"
#include "SharedMaterial.h"
#include "Texture.h"
#include "Shader.h"
#include "RenderManager/VulkanAllocator.h"

namespace JoyEngine {

//    class IJoyGraphicsContext;

//    class JoyGraphicsContext;

    class ResourceManager {
    public:

        ResourceManager() = default;

        void Init() {}

        void Start() {}

        void Stop() {}

        bool IsResourceLoaded(GUID guid) {
            return m_loadedResources.find(guid) != m_loadedResources.end();
        }

        template<class T>
        void LoadResource(GUID guid) {
            if (IsResourceLoaded(guid)) {
                m_loadedResources[guid]->IncreaseRefCount();
                return;
            }
            m_loadedResources.insert({guid, std::make_unique<T>(guid)});
            m_loadedResources[guid]->IncreaseRefCount();
        }

        void UnloadResource(GUID guid) {
            if (IsResourceLoaded(guid)) {
                m_loadedResources[guid]->DecreaseRefCount();
            } else {
                ASSERT(false);
            }
            if (m_loadedResources[guid]->GetRefCount() == 0) {
                m_loadedResources.erase(guid);
            }
        }

        template<class T>
        T *GetResource(GUID guid) {
            ASSERT(IsResourceLoaded(guid));
#ifdef DEBUG
            T *ptr = dynamic_cast<T *>(m_loadedResources[guid].get());
            ASSERT(ptr != nullptr);
#else
            T *ptr = reinterpret_cast<T *>(m_loadedResources[guid].get());
#endif //DEBUG
            return ptr;
        }

    private:
        std::map<GUID, std::unique_ptr<Resource>> m_loadedResources;
    };
}

#endif //RESOURCE_MANAGER_H