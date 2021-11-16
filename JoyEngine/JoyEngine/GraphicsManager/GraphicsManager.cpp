#include "GraphicsManager/GraphicsManager.h"

#include "RenderManager/VulkanAllocator.h"
#include "RenderManager/VulkanUtils.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	GraphicsManager::GraphicsManager(HINSTANCE instance, HWND windowHandle) :
		m_windowInstance(instance),
		m_windowHandle(windowHandle)
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		FindQueueFamilies();
		CreateLogicalDevice();
		CreateCommandPool();
	}

	GraphicsManager::~GraphicsManager()
	{
		vkDestroyCommandPool(m_logicalDevice, m_commandPool, m_allocator->GetAllocationCallbacks());
		vkDestroyDevice(m_logicalDevice, m_allocator->GetAllocationCallbacks());

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, m_allocator->GetAllocationCallbacks());
		}

		vkDestroySurfaceKHR(m_vkInstance, m_surface, m_allocator->GetAllocationCallbacks());
		vkDestroyInstance(m_vkInstance, m_allocator->GetAllocationCallbacks());
	}

	void GraphicsManager::CreateInstance()
	{
		m_allocator = new Allocator();
		if (enableValidationLayers && !checkValidationLayerSupport(validationLayerName))
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			nullptr,
			"Joy Instance",
			VK_MAKE_VERSION(1, 0, 0),
			"Joy Engine",
			VK_MAKE_VERSION(1, 0, 0),
			VK_API_VERSION_1_0
		};
		auto extensions = getRequiredExtensions(enableValidationLayers);

		VkInstanceCreateInfo createInfo{
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			nullptr,
			0,
			&appInfo,
			0,
			nullptr,
			static_cast<uint32_t>(extensions.size()),
			extensions.data()
		};

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = 1;
			createInfo.ppEnabledLayerNames = &validationLayerName;

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}

		VkResult res = vkCreateInstance(&createInfo, m_allocator->GetAllocationCallbacks(), &m_vkInstance);
		ASSERT(res == VK_SUCCESS);
	}

	void GraphicsManager::SetupDebugMessenger()
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, m_allocator->GetAllocationCallbacks(),
		                                 &m_debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void GraphicsManager::CreateSurface()
	{
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfoKhr = {
			VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			nullptr,
			0,
			m_windowInstance,
			m_windowHandle,
		};
		VkResult res = vkCreateWin32SurfaceKHR(m_vkInstance, &surfaceCreateInfoKhr,
		                                       m_allocator->GetAllocationCallbacks(), &m_surface);
		if (res != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}


	void GraphicsManager::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());
		for (const auto& device : devices)
		{
			if (isPhysicalDeviceSuitable(device, m_surface, deviceExtensions))
			{
				m_physicalDevice = device;
				break;
			}
		}
		if (m_physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	void GraphicsManager::FindQueueFamilies()
	{
		m_queueFamilyIndices = std::make_unique<QueueFamilyIndices>();
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

#ifdef DEBUG
		for (int i = 0; i < queueFamilies.size(); i++)
		{
			std::string s = "Queue " + std::to_string(i) + ": size = " + std::to_string(queueFamilies[i].queueCount) + "\n";
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) s += "    VK_QUEUE_GRAPHICS_BIT \n";
			if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) s += "    VK_QUEUE_COMPUTE_BIT \n";
			if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) s += "    VK_QUEUE_TRANSFER_BIT \n";
			OutputDebugStringA(s.c_str());
		}
#endif


		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				m_queueFamilyIndices->graphicsFamily = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				m_queueFamilyIndices->transferFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);

			if (presentSupport)
			{
				m_queueFamilyIndices->presentFamily = i;
			}

			if (m_queueFamilyIndices->isComplete())
			{
				break;
			}

			i++;
		}

		ASSERT(m_queueFamilyIndices->isComplete());
	}

	void GraphicsManager::CreateLogicalDevice()
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {
			m_queueFamilyIndices->graphicsFamily.value(),
			m_queueFamilyIndices->presentFamily.value(),
			m_queueFamilyIndices->transferFamily.value()
		};

		float queuePriority[] = {1.0f, 1.0f};
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{
				VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				nullptr,
				0,
				queueFamily,
				2,
				queuePriority
			};

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		memset(&deviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(queueCreateInfos.size()),
			queueCreateInfos.data(),
			enableValidationLayers ? static_cast<uint32_t>(1) : 0,
			enableValidationLayers ? &validationLayerName : nullptr,
			static_cast<uint32_t>(deviceExtensions.size()),
			deviceExtensions.data(),
			&deviceFeatures
		};

		VkResult res = vkCreateDevice(m_physicalDevice, &createInfo, m_allocator->GetAllocationCallbacks(),
		                              &m_logicalDevice);
		ASSERT(res == VK_SUCCESS);

		// graphics and presentation operation will run in a row, but transfer operation are in parallel queue/thread
		vkGetDeviceQueue(m_logicalDevice, m_queueFamilyIndices->graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_logicalDevice, m_queueFamilyIndices->presentFamily.value(), 0, &m_presentQueue);
		vkGetDeviceQueue(m_logicalDevice, m_queueFamilyIndices->presentFamily.value(), 1, &m_transferQueue);
	}

	void GraphicsManager::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_queueFamilyIndices->graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
		if (vkCreateCommandPool(m_logicalDevice, &poolInfo, m_allocator->GetAllocationCallbacks(), &m_commandPool) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}
}
