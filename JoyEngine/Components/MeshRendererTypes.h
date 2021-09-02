#ifndef MESH_RENDERER_TYPES_H
#define MESH_RENDERER_TYPES_H

#include "Utils/GUID.h"

namespace JoyEngine {

    class Resource {
    public:
        [[nodiscard]] GUID GetGuid() const noexcept { return m_guid; }

    protected:
        GUID m_guid;
    };

    class Material : public Resource {
    public :
        Material(GUID guid, const std::string &filename);

        ~Material();

    };

    class Mesh : public Resource {
    public :
        Mesh(GUID guid, const std::string &filename);

        ~Mesh();

    };

    class Texture : public Resource {
    public :
        Texture(GUID guid, const std::string &filename);

        ~Texture();
    };

    class Shader : public Resource {
    public :
        Shader(GUID guid, const std::string &filename);

        ~Shader();
    };
}

#endif //MESH_RENDERER_TYPES_H
