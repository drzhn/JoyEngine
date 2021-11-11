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

    	[[nodiscard]] bool IsLoaded() const noexcept override;
    private:
        void CreateDescriptorSets();

        void LoadResources() const;
    
    private :
        GUID m_sharedMaterialGuid;
        SharedMaterial *m_sharedMaterial = nullptr;
        std::map<std::string, GUID> m_bindings;

        std::map<uint32_t, std::vector<VkDescriptorSet>> m_descriptorSets;

        
    };
}

#endif //MATERIAL_H