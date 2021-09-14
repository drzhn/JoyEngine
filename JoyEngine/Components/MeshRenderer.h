#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include <optional>

#include "Component.h"
#include "Utils/GUID.h"
#include "Utils/Assert.h"
#include "ResourceManager/Mesh.h"
#include "ResourceManager/Material.h"

namespace JoyEngine {
    class MeshRenderer : public Component {
    public:
        MeshRenderer() = delete;

        explicit MeshRenderer(Transform *t);

        void Enable() final;

        void Disable() final;

        ~MeshRenderer();

        void SetMesh(GUID meshGuid);

        void SetMaterial(GUID materialGuid);

        [[nodiscard]]GUID GetMeshGuid() const noexcept;

        [[nodiscard]]GUID GetMaterialGuid() const noexcept;

        [[nodiscard]]mesh *GetMesh() const noexcept;

        [[nodiscard]]Material *GetMaterial() const noexcept;

    private:
        uint32_t m_index;
        std::optional<GUID> m_meshGuid;
        std::optional<GUID> m_materialGuid;
    };
}


#endif //MESHRENDERER_H
