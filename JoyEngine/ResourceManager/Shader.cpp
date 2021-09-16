#include "Shader.h"

#include "JoyContext.h"

#include <vector>

#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine {

    Shader::Shader(GUID guid) {
        std::vector<char> shaderData = JoyContext::Data()->GetData<char>(guid);
        JoyContext::Memory()->CreateShaderModule(reinterpret_cast<const uint32_t *>(shaderData.data()),
                                                         shaderData.size(),
                                                         m_shaderModule);
    }

    Shader::~Shader() {
        JoyContext::Memory()->DestroyShaderModule(m_shaderModule);
    }

}
