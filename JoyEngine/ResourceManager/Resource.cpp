#include "Resource.h"

#include <string>
#include <fstream>
#include <Utils/ModelLoader.h>

#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/ResourceManager.h"

namespace JoyEngine {

    SharedMaterial::SharedMaterial(GUID guid) {
        DataManager::GetInstance()->ParseSharedMaterial(guid,
                                                        m_vertexShader,
                                                        m_fragmentShader,
                                                        m_hasVertexInput,
                                                        m_hasMVP,
                                                        m_depthTest,
                                                        m_depthWrite,
                                                        m_bindingSets
        );
    }

    SharedMaterial::~SharedMaterial() {

    }

    Shader *SharedMaterial::GetVertexShader() const noexcept {
        return ResourceManager::GetInstance()->GetResource<Shader>(m_vertexShader);
    }

    Shader *SharedMaterial::GetFragmentShader() const noexcept {
        return ResourceManager::GetInstance()->GetResource<Shader>(m_fragmentShader);
    }

    Material::Material(GUID guid) {
        DataManager::GetInstance()->ParseMaterial(guid, m_sharedMaterialGuid, m_bindings);
        ResourceManager::GetInstance()->LoadResource<SharedMaterial>(m_sharedMaterialGuid);
    }

    Material::~Material() {
        ResourceManager::GetInstance()->UnloadResource(m_sharedMaterialGuid);
        for (const auto &item : m_bindings)
        {
            ResourceManager::GetInstance()->UnloadResource(item.second);
        }
    }

    SharedMaterial *Material::GetSharedMaterial() const noexcept {
        return ResourceManager::GetInstance()->GetResource<SharedMaterial>(m_sharedMaterialGuid);
    }

    Mesh::Mesh(GUID guid) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::ifstream modelStream;
        DataManager::GetInstance()->GetDataStream(modelStream, guid);
        ModelLoader::LoadModel(vertices, indices, modelStream);
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
        if (modelStream.is_open()) {
            modelStream.close();
        }
    }

    Mesh::~Mesh() {
        MemoryManager::GetInstance()->DestroyBuffer(m_vertexBuffer, m_vertexBufferMemory);
        MemoryManager::GetInstance()->DestroyBuffer(m_indexBuffer, m_indexBufferMemory);
    }

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