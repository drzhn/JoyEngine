#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>
#include <string>

#include "Resource.h"
#include "SharedMaterial.h"
#include "Utils/GUID.h"

namespace JoyEngine {
    class Material final: public Resource {
    public :
        Material() = delete;

        explicit Material(GUID);

        ~Material() final;

        [[nodiscard]] SharedMaterial *GetSharedMaterial() const noexcept;

    private :
        GUID m_sharedMaterialGuid;
        std::map <std::string, GUID> m_bindings;
    };
}

#endif //MATERIAL_H
