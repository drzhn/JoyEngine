#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>

#include "MemoryManager/MemoryManager.h"
#include "SceneManager/SceneManager.h"
#include "SceneManager/GameObject.h"
#include "RenderManager/RenderManager.h"
#include "RenderManager/RenderObject.h"

namespace JoyEngine {

    class ResourceManager {
    public:
        ResourceManager() = default;
        ResourceManager(MemoryManager& memoryManager):m_memoryManager(memoryManager){}
        void Init(){}
        void Start(){}
        void Stop(){}

    private:
        const MemoryManager& m_memoryManager;
    };
}

#endif //RESOURCE_MANAGER_H