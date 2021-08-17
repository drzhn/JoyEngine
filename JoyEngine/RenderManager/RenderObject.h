#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include <vulkan/vulkan.h>

#include "Components/MeshRenderer.h"
#include "Components/MeshRendererTypes.h"

namespace JoyEngine {
    class MeshRenderer;

    class RenderObject {
    public :
        RenderObject() = delete;
        explicit RenderObject(MeshRenderer* meshRenderer);
    private:
        MeshRenderer * m_meshRenderer;
    };
}


#endif //RENDER_OBJECT_H
