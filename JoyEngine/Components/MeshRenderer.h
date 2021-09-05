#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include <optional>

#include "Component.h"
#include "SceneManager/Transform.h"
#include "Utils/GUID.h"

namespace JoyEngine {
    class MeshRenderer : public Component {
    public:
        MeshRenderer() = delete;

        explicit MeshRenderer(Transform t);

        void Enable() final;

        void Disable() final;

        ~MeshRenderer();

        void SetMesh(GUID meshGuid);

        void SetMaterial(GUID materialGuid);

//        [[nodiscard]]Mesh *GetMesh() const noexcept { return m_mesh.get(); }

    private:
        const Transform &m_transform;
        uint32_t m_index;
        std::optional<GUID> m_meshGuid;
        std::optional<GUID> m_materialGuid;
    };
}


#endif //MESHRENDERER_H
