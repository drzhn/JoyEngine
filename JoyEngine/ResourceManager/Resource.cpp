#include "Resource.h"

#include <string>
#include <Utils/ModelLoader.h>

#include "MemoryManager/MemoryManager.h"

namespace JoyEngine {

    Mesh::Mesh(const std::string &filename) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        ModelLoader::LoadModel(vertices, indices, filename.c_str());
        m_vertexSize = vertices.size();
        m_indexSize = indices.size();
        MemoryManager::GetInstance()->CreateGPUBuffer(vertices.data(),
                                                      sizeof(Vertex),
                                                      vertices.size(),
                                                      m_vertexBuffer,
                                                      m_vertexBufferMemory,
                                                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        MemoryManager::GetInstance()->CreateGPUBuffer(indices.data(),
                                                      sizeof(uint32_t),
                                                      indices.size(),
                                                      m_indexBuffer,
                                                      m_indexBufferMemory,
                                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

    Mesh::~Mesh() {
        MemoryManager::GetInstance()->DestroyBuffer(m_vertexBuffer, m_vertexBufferMemory);
        MemoryManager::GetInstance()->DestroyBuffer(m_indexBuffer, m_indexBufferMemory);
    }

    Texture::Texture(const std::string &filename) {
        MemoryManager::GetInstance()->CreateTexture(m_textureImage, m_textureImageView, m_textureImageMemory, filename);
        MemoryManager::GetInstance()->CreateTextureSampler(m_textureSampler);
    }

    Texture::~Texture() {
        if (m_textureSampler != VK_NULL_HANDLE)
        {
            MemoryManager::GetInstance()->DestroySampler(m_textureSampler);
        }
        MemoryManager::GetInstance()->DestroyImage(m_textureImageView,
                                                   m_textureImage,
                                                   m_textureImageMemory);
    }

    Shader::Shader(const std::string &filename) {
        MemoryManager::GetInstance()->CreateShaderModule(filename, m_shaderModule);
    }

    Shader::~Shader() {
        MemoryManager::GetInstance()->DestroyShaderModule(m_shaderModule);
    }
}