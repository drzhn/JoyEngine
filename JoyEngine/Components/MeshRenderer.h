#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "Component.h"
#include "MeshRendererTypes.h"
#include "SceneManager/Transform.h"
#include "Utils/GUID.h"

namespace JoyEngine {
    class MeshRenderer : public Component {
    public:
        explicit MeshRenderer(Transform t) : m_transform(t) {

        }

        void Enable() override {
            // Register in render list
        }

        void Disable() override {
            // Unregister in render list
        };

        ~MeshRenderer() {
            // if enabled - disable
            // tell resource manager to free resources;
        }

        void SetMesh(const char *filename, GUID guid) {

        }

        void SetTexture(const char *filename, GUID guid) {

        }

        void SetVertShader(const char *filename, GUID guid) {

        }

        void SetFragShader(const char *filename, GUID guid) {

        }

    private:
        const Transform &m_transform;
        uint32_t m_index;
        //Mesh m_mesh;
        //Texture m_texture;
        //Shader m_vertShader;
        //Shader m_fragShader;
    };
}


#endif //MESHRENDERER_H
