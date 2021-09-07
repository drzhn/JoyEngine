#include "Texture.h"

#include <vector>
#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine {
    Texture::Texture(GUID guid) {
        std::vector<unsigned char> imageData = DataManager::GetInstance()->GetData<unsigned char>(guid);
        MemoryManager::GetInstance()->CreateTexture(m_textureImage,
                                                    m_textureImageView,
                                                    m_textureImageMemory,
                                                    imageData.data(),
                                                    static_cast<int>(imageData.size()));
        MemoryManager::GetInstance()->CreateTextureSampler(m_textureSampler);
    }

    Texture::~Texture() {
        if (m_textureSampler != VK_NULL_HANDLE) {
            MemoryManager::GetInstance()->DestroySampler(m_textureSampler);
        }
        MemoryManager::GetInstance()->DestroyImage(m_textureImageView,
                                                   m_textureImage,
                                                   m_textureImageMemory);
    }
}