#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "Component.h"
#include "MeshRendererTypes.h"
#include "SceneManager/Transform.h"
#include "Utils/GUID.h"

namespace JoyEngine {
    class MeshRenderer : public Component {
    public:
        MeshRenderer() = default;

        explicit MeshRenderer(Transform t) : m_transform(t) {

        }

        void Enable() final;

        void Disable() final;

        ~MeshRenderer();

        void SetMesh(const char *filename, GUID guid) {
            m_mesh = new Mesh(guid, filename);
        }

        void SetTexture(const char *filename, GUID guid) {
            m_texture = new Texture(guid, filename);
        }

        void SetVertShader(const char *filename, GUID guid) {
            m_vertShader = new Shader(guid, filename);
        }

        void SetFragShader(const char *filename, GUID guid) {
            m_fragShader = new Shader(guid, filename);
        }

        [[nodiscard]]Mesh *GetMesh() const noexcept { return m_mesh; }

        [[nodiscard]]Texture *GetTexture() const noexcept { return m_texture; }

        [[nodiscard]]Shader *GetVertexShader() const noexcept { return m_vertShader; }

        [[nodiscard]]Shader *GetFragmentShader() const noexcept { return m_fragShader; }

    private:
        const Transform &m_transform;
        uint32_t m_index;
        Mesh *m_mesh;
        Texture *m_texture;
        Shader *m_vertShader;
        Shader *m_fragShader;
    };
}


#endif //MESHRENDERER_H
