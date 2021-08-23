#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <set>
#include <cassert>
#include <memory>
#include <vulkan/vulkan.h>

#include "IJoyGraphicsContext.h"

#include "Components/MeshRendererTypes.h"
#include "GFXResource.h"
#include "RenderManager/VulkanAllocator.h"

namespace JoyEngine {

    class IJoyGraphicsContext;

    class JoyGraphicsContext;

    class ResourceManager {
    public:

        ResourceManager() = default;

        ResourceManager(IJoyGraphicsContext *const);

        static ResourceManager *GetInstance() noexcept {
            assert(m_instance != nullptr);
            return m_instance;
        }

        void Init() {}

        void Start() {}

        void Stop() {}

        template<class T>
        void LoadResource(GUID guid, const std::string &filename) { assert(false); }

        // implementation is here because of https://stackoverflow.com/questions/456713/why-do-i-get-unresolved-external-symbol-errors-when-using-templates
        template<>
        void LoadResource<Mesh>(GUID guid, const std::string &filename) {
            if (m_loadedMeshes.find(guid) != m_loadedMeshes.end()) {
                m_loadedMeshes[guid]->IncreaseRefCount();
                return;
            }
            m_loadedMeshes.insert({guid, std::make_unique<GFXMesh>(filename)});
            m_loadedMeshes[guid]->IncreaseRefCount();
        }

        template<>
        void LoadResource<Texture>(GUID guid, const std::string &filename) {
            if (m_loadedTextures.find(guid) != m_loadedTextures.end()) {
                m_loadedTextures[guid]->IncreaseRefCount();
                return;
            }
            m_loadedTextures.insert({guid, std::make_unique<GFXTexture>(filename)});
            m_loadedTextures[guid]->IncreaseRefCount();
        }

        template<>
        void LoadResource<Shader>(GUID guid, const std::string &filename) {
            if (m_loadedShaders.find(guid) != m_loadedShaders.end()) {
                m_loadedShaders[guid]->IncreaseRefCount();
                return;
            }
            m_loadedShaders.insert({guid, std::make_unique<GFXShader>(filename)});
            m_loadedShaders[guid]->IncreaseRefCount();
        }

        template<class T>
        void UnloadResource(GUID guid) { assert(false); }

        template<>
        void UnloadResource<Texture>(GUID guid) {
            if (m_loadedTextures.find(guid) != m_loadedTextures.end()) {
                m_loadedTextures[guid]->DecreaseRefCount();
            } else {
                assert(false);
            }
            if (m_loadedTextures[guid]->GetRefCount() == 0) {
                m_loadedTextures.erase(guid);
            }
        }

        template<>
        void UnloadResource<Mesh>(GUID guid) {
            if (m_loadedMeshes.find(guid) != m_loadedMeshes.end()) {
                m_loadedMeshes[guid]->DecreaseRefCount();
            }
            if (m_loadedMeshes[guid]->GetRefCount() == 0) {
                m_loadedMeshes.erase(guid);
            }
        }

        template<>
        void UnloadResource<Shader>(GUID guid) {
            if (m_loadedShaders.find(guid) != m_loadedShaders.end()) {
                m_loadedShaders[guid]->DecreaseRefCount();
            } else {
                assert(false);
            }
            if (m_loadedShaders[guid]->GetRefCount() == 0) {
                m_loadedShaders.erase(guid);
            }
        }


        GFXMesh *GetMesh(GUID guid) { return m_loadedMeshes[guid].get(); };

        GFXTexture *GetTexture(GUID guid) { return m_loadedTextures[guid].get(); };

        GFXShader *GetShader(GUID guid) { return m_loadedShaders[guid].get(); };

    private:
        static ResourceManager *m_instance;

        IJoyGraphicsContext *const m_graphicsContext;
        const VkAllocationCallbacks *m_allocator;

        std::map<GUID, std::unique_ptr<GFXMesh>> m_loadedMeshes;
        std::map<GUID, std::unique_ptr<GFXTexture>> m_loadedTextures;
        std::map<GUID, std::unique_ptr<GFXShader>> m_loadedShaders;


    };
}

#endif //RESOURCE_MANAGER_H