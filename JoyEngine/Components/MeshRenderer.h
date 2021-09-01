#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include <memory>

#include "Component.h"
#include "MeshRendererTypes.h"
#include "SceneManager/Transform.h"
#include "Utils/GUID.h"

namespace JoyEngine {
    class MeshRenderer : public Component {
    public:
        MeshRenderer() = delete;

        explicit MeshRenderer(Transform t) : m_transform(t) {

        }

        void Enable() final;

        void Disable() final;

        ~MeshRenderer();

        void SetMesh(const char *filename, GUID guid) {
            m_mesh = std::make_unique<Mesh>(guid, filename);
        }

        void SetTexture(const char *filename, GUID guid) {
            m_texture = std::make_unique<Texture>(guid, filename);
        }

        void SetVertShader(const char *filename, GUID guid) {
            m_vertShader = std::make_unique<Shader>(guid, filename);
        }

        void SetFragShader(const char *filename, GUID guid) {
            m_fragShader = std::make_unique<Shader>(guid, filename);
        }

        [[nodiscard]]Mesh *GetMesh() const noexcept { return m_mesh.get(); }

        [[nodiscard]]Texture *GetTexture() const noexcept { return m_texture.get(); }

        [[nodiscard]]Shader *GetVertexShader() const noexcept { return m_vertShader.get(); }

        [[nodiscard]]Shader *GetFragmentShader() const noexcept { return m_fragShader.get(); }

    private:
        const Transform &m_transform;
        uint32_t m_index;
        std::unique_ptr<Mesh> m_mesh;
        std::unique_ptr<Texture> m_texture;
        std::unique_ptr<Shader> m_vertShader;
        std::unique_ptr<Shader> m_fragShader;
    };
}


#endif //MESHRENDERER_H
