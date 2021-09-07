#include "Shader.h"

namespace JoyEngine {

    Shader::Shader(GUID guid) {
        std::vector<char> shaderData = DataManager::GetInstance()->GetData<char>(guid);
        MemoryManager::GetInstance()->CreateShaderModule(reinterpret_cast<const uint32_t *>(shaderData.data()),
                                                         shaderData.size(),
                                                         m_shaderModule);
    }

    Shader::~Shader() {
        MemoryManager::GetInstance()->DestroyShaderModule(m_shaderModule);
    }

}
