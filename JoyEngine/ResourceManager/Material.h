#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>
#include <string>

#include "Common/Resource.h"
#include "SharedMaterial.h"
#include "Utils/GUID.h"

namespace JoyEngine {
    class Material final : public Resource {
    public :
        Material() = delete;

        explicit Material(GUID);

        ~Material() final;

        [[nodiscard]] SharedMaterial *GetSharedMaterial() const noexcept;

        [[nodiscard]] std::map<uint32_t, std::vector<VkDescriptorSet>> GetDescriptorMap() const noexcept;

    private:
        void CreateDescriptorSets();

        void LoadResources();

    private :
        GUID m_sharedMaterialGuid;
        SharedMaterial *m_sharedMaterial;
        std::map<std::string, GUID> m_bindings;

        std::map<uint32_t, std::vector<VkDescriptorSet>> m_descriptorSets;

        
    };
}

#endif //MATERIAL_H
