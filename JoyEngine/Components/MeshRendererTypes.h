#ifndef MESH_RENDERER_TYPES_H
#define MESH_RENDERER_TYPES_H

namespace JoyEngine {
    class Resource {
    public:
        virtual void Load() = 0;

        virtual void Unload() = 0;
    };

    class Mesh : public Resource {

    };

    class Texture : public Resource {

    };

    class Shader : public Resource {

    };
}

#endif //MESH_RENDERER_TYPES_H
