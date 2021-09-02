#include "MeshRendererTypes.h"

#include <string>
#include "ResourceManager/ResourceManager.h"

namespace JoyEngine {

    Material::Material(GUID guid, const std::string &filename) {
        m_guid = guid;
        ResourceManager::GetInstance()->LoadResource<Material>(guid, filename);
    }

    Material::~Material() {
        ResourceManager::GetInstance()->UnloadResource<Material>(m_guid);
    }

    Mesh::Mesh(GUID guid, const std::string &filename) {
        m_guid = guid;
        ResourceManager::GetInstance()->LoadResource<Mesh>(guid, filename);
    }

    Mesh::~Mesh() {
        ResourceManager::GetInstance()->UnloadResource<Mesh>(m_guid);
    }

    Texture::Texture(GUID guid, const std::string &filename) {
        m_guid = guid;
        ResourceManager::GetInstance()->LoadResource<Texture>(guid, filename);
    }

    Texture::~Texture() {
        ResourceManager::GetInstance()->UnloadResource<Texture>(m_guid);
    }

    Shader::Shader(GUID guid, const std::string &filename) {
        m_guid = guid;
        ResourceManager::GetInstance()->LoadResource<Shader>(guid, filename);
    }

    Shader::~Shader() {
        ResourceManager::GetInstance()->UnloadResource<Shader>(m_guid);
    }

}
