#ifndef SHADER_H
#define SHADER_H

#include <vulkan/vulkan.h>

#include "Common/Resource.h"

namespace JoyEngine {
    class Shader final: public Resource {
    public :

        Shader() = delete;

        explicit Shader(GUID);

        ~Shader() final;

        [[nodiscard]] VkShaderModule &GetVertexShadeModule() noexcept { return m_vertexModule; }
        [[nodiscard]] VkShaderModule &GetFragmentShadeModule() noexcept { return m_fragmentModule; }
        [[nodiscard]] bool IsLoaded() const noexcept override { return true; }

    private :
        VkShaderModule m_vertexModule = VK_NULL_HANDLE;
        VkShaderModule m_fragmentModule = VK_NULL_HANDLE;
    };
}

#endif //SHADER_H
