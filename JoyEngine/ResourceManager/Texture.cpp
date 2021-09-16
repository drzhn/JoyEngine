#include "Texture.h"

#include "JoyContext.h"

#include <vector>

#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine {
    Texture::Texture(GUID guid) {
        std::vector<unsigned char> imageData = JoyContext::Data()->GetData<unsigned char>(guid);
        JoyContext::Memory()->CreateTexture(m_textureImage,
                                                    m_textureImageView,
                                                    m_textureImageMemory,
                                                    imageData.data(),
                                                    static_cast<int>(imageData.size()));
        JoyContext::Memory()->CreateTextureSampler(m_textureSampler);
    }

    Texture::~Texture() {
        if (m_textureSampler != VK_NULL_HANDLE) {
            JoyContext::Memory()->DestroySampler(m_textureSampler);
        }
        JoyContext::Memory()->DestroyImage(m_textureImageView,
                                                   m_textureImage,
                                                   m_textureImageMemory);
    }
}