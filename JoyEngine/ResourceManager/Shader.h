#ifndef SHADER_H
#define SHADER_H

#include "Common/Resource.h"

namespace JoyEngine {
    class Shader final: public Resource {
    public :

        Shader() = delete;

        explicit Shader(GUID);

        ~Shader() final;

        [[nodiscard]] VkShaderModule &GetShadeModule() noexcept { return m_shaderModule; }

    private :
        VkShaderModule m_shaderModule = VK_NULL_HANDLE;
    };
}

#endif //SHADER_H
