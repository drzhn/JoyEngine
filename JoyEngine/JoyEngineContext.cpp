#include "JoyEngineContext.h"
#include "RenderManager/VulkanUtils.h"

namespace JoyEngine {
    JoyGraphicsContext::JoyGraphicsContext(HINSTANCE instance, HWND windowHandle) :
            m_windowInstance(instance),
            m_windowHandle(windowHandle) {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateCommandPool();

        CreateSwapChain();

        CreateImageViews();
    }

    JoyGraphicsContext::~JoyGraphicsContext() {
        vkDestroyCommandPool(m_logicalDevice, m_commandPool, m_allocator->GetAllocationCallbacks());
        vkDestroyDevice(m_logicalDevice, m_allocator->GetAllocationCallbacks());

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, m_allocator->GetAllocationCallbacks());
        }

        vkDestroySurfaceKHR(m_vkInstance, m_surface, m_allocator->GetAllocationCallbacks());
        vkDestroyInstance(m_vkInstance, m_allocator->GetAllocationCallbacks());
    }

    void JoyGraphicsContext::CreateInstance() {
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

    void JoyGraphicsContext::SetupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, m_allocator->GetAllocationCallbacks(), &m_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void JoyGraphicsContext::CreateSurface() {
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


    void JoyGraphicsContext::PickPhysicalDevice() {
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

    void JoyGraphicsContext::CreateLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
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

        vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue);
    }

    void JoyGraphicsContext::CreateCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice, m_surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = 0; // Optional
        if (vkCreateCommandPool(m_logicalDevice, &poolInfo, m_allocator->GetAllocationCallbacks(), &m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void JoyGraphicsContext::CreateSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(
                m_physicalDevice,
                m_surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, m_windowHandle);

        m_swapchainImageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && m_swapchainImageCount > swapChainSupport.capabilities.maxImageCount) {
            m_swapchainImageCount = swapChainSupport.capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;

        createInfo.minImageCount = m_swapchainImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices queueFamilies = findQueueFamilies(m_physicalDevice, m_surface);
        uint32_t queueFamilyIndices[] = {queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value()};

        if (queueFamilies.graphicsFamily != queueFamilies.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_logicalDevice, &createInfo, m_allocator->GetAllocationCallbacks(), &m_swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &m_swapchainImageCount, nullptr);
        m_swapChainImages.resize(m_swapchainImageCount);
        vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &m_swapchainImageCount, m_swapChainImages.data());

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;
    }

    void JoyGraphicsContext::CreateImageViews() {
        m_swapChainImageViews.resize(m_swapchainImageCount);

        for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
            m_swapChainImageViews[i] = CreateImageView(m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    VkImageView JoyGraphicsContext::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(m_logicalDevice, &viewInfo, m_allocator->GetAllocationCallbacks(), &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

}
