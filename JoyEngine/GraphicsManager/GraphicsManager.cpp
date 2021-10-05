#include "GraphicsManager/GraphicsManager.h"

#include "RenderManager/VulkanAllocator.h"
#include "RenderManager/VulkanUtils.h"

namespace JoyEngine {

    GraphicsManager::GraphicsManager(HINSTANCE instance, HWND windowHandle) :
            m_windowInstance(instance),
            m_windowHandle(windowHandle) {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateCommandPool();
    }

    GraphicsManager::~GraphicsManager() {
        vkDestroyCommandPool(m_logicalDevice, m_commandPool, m_allocator->GetAllocationCallbacks());
        vkDestroyDevice(m_logicalDevice, m_allocator->GetAllocationCallbacks());

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, m_allocator->GetAllocationCallbacks());
        }

        vkDestroySurfaceKHR(m_vkInstance, m_surface, m_allocator->GetAllocationCallbacks());
        vkDestroyInstance(m_vkInstance, m_allocator->GetAllocationCallbacks());
    }

    void GraphicsManager::CreateInstance() {
        m_allocator = new Allocator();
        if (enableValidationLayers && !checkValidationLayerSupport(validationLayerName)) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Joy Instance";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Joy Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions(enableValidationLayers);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = &validationLayerName;

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = m_allocator->GetAllocationCallbacks();
        }

        if (vkCreateInstance(&createInfo, m_allocator->GetAllocationCallbacks(), &m_vkInstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void GraphicsManager::SetupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, m_allocator->GetAllocationCallbacks(), &m_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void GraphicsManager::CreateSurface() {
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfoKhr = {
                VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                nullptr,
                0,
                m_windowInstance,
                m_windowHandle,
        };
        VkResult res = vkCreateWin32SurfaceKHR(m_vkInstance, &surfaceCreateInfoKhr, m_allocator->GetAllocationCallbacks(), &m_surface);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }


    void GraphicsManager::PickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());
        for (const auto &device : devices) {
            if (isPhysicalDeviceSuitable(device, m_surface, deviceExtensions)) {
                m_physicalDevice = device;
                break;
            }
        }
        if (m_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void GraphicsManager::CreateLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };

        float queuePriority[] = { 1.0f,1.0f };
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 2;
            queueCreateInfo.pQueuePriorities = queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = &validationLayerName;
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_physicalDevice, &createInfo, m_allocator->GetAllocationCallbacks(), &m_logicalDevice) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        // graphics and presentation operation will run in a row, but transfer operation are in parallel queue/thread
        vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue);
        vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 1, &m_transferQueue); 
    }

    void GraphicsManager::CreateCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice, m_surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
        if (vkCreateCommandPool(m_logicalDevice, &poolInfo, m_allocator->GetAllocationCallbacks(), &m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
}
