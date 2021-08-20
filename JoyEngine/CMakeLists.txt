cmake_minimum_required(VERSION 3.8)
project(JoyEngine)

set(CMAKE_CXX_STANDARD 17)

add_library(
        JoyEngine

        JoyGraphicsContext.cpp

        RenderManager/RenderManager.cpp
        MemoryManager/MemoryManager.cpp
        ResourceManager/ResourceManager.cpp
        SceneManager/SceneManager.cpp
        SceneManager/Scene.cpp
        SceneManager/GameObject.cpp
        SceneManager/Transform.cpp
        Components/MeshRenderer.cpp
        RenderManager/RenderObject.cpp
        Components/MeshRendererTypes.cpp
        RenderManager/Swapchain.cpp
        ResourceManager/GFXResource.cpp)
############## make #include "<folder>/<header>.h" possible ##############
target_include_directories(JoyEngine PUBLIC .)


############################ LIBRARIES ############################

target_link_libraries(JoyEngine C://VulkanSDK/1.2.182.0/Lib/vulkan-1.lib)
target_include_directories(JoyEngine PUBLIC C://VulkanSDK/1.2.182.0/Include)
target_include_directories(JoyEngine PUBLIC ${CMAKE_SOURCE_DIR})
add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
add_compile_definitions(VK_PROTOTYPES)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_USE_MATH_DEFINES)
#add_compile_definitions(STB_IMAGE_IMPLEMENTATION)


add_subdirectory(Libs/glm)
target_link_libraries(JoyEngine glm)

add_subdirectory(Libs/tinyobjloader)
target_link_libraries(JoyEngine tinyobjloader)

target_include_directories(JoyEngine PUBLIC Libs/stb) # somehow this is not a library

add_subdirectory(Libs/rapidjson)
target_include_directories(JoyEngine PUBLIC Libs/rapidjson/include)

############################ TEST ############################

add_executable(RenderTest Tests/Test.cpp)
target_include_directories(RenderTest PUBLIC .)
target_include_directories(RenderTest PUBLIC Libs/rapidjson/include)

