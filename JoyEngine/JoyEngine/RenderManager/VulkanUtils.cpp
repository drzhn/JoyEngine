#include "VulkanUtils.h"

#include "windows.h"

#include <set>
#include <string>

#include "Utils/Assert.h"

namespace JoyEngine
{
	VkResult
	CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	                             const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
	                                   const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			//std::cout << "Validation Warning: " << pCallbackData->pMessage << std::endl;
			OutputDebugStringA("Warning: Warning vulkan validation: ");
		}
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			//std::cerr << "validation Error: " << pCallbackData->pMessage << std::endl;
			OutputDebugStringA("Error: Error vulkan validation: ");
		}
		OutputDebugStringA(pCallbackData->pMessage);
		OutputDebugStringA("\n");

		return VK_FALSE;
	}

	bool checkValidationLayerSupport(const char* validationLayerName)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(validationLayerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}

		return true;
	}

	std::vector<const char*> getRequiredExtensions(bool enableValidationLayers)
	{
		std::vector<const char*> extensions;
		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);


		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			//                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void ParseQueueFamilyProperties(const std::vector<VkQueueFamilyProperties>& props)
	{
		for (int i = 0; i < props.size(); i++)
		{
			std::string s = "Queue " + std::to_string(i) + ": size = " + std::to_string(props[i].queueCount) + "\n";
			if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) s += "    VK_QUEUE_GRAPHICS_BIT \n";
			if (props[i].queueFlags & VK_QUEUE_COMPUTE_BIT) s += "    VK_QUEUE_COMPUTE_BIT \n";
			if (props[i].queueFlags & VK_QUEUE_TRANSFER_BIT) s += "    VK_QUEUE_TRANSFER_BIT \n";
			OutputDebugStringA(s.c_str());
		}
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surfaceKhr)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

#ifdef DEBUG
		ParseQueueFamilyProperties(queueFamilies);
#endif


		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				indices.transferFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surfaceKhr, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surfaceKhr)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surfaceKhr, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surfaceKhr, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surfaceKhr, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surfaceKhr, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surfaceKhr, &presentModeCount,
			                                          details.presentModes.data());
		}

		return details;
	}

	bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surfaceKhr,
	                              std::vector<const char*> deviceExtensions)
	{
		QueueFamilyIndices indices = findQueueFamilies(device, surfaceKhr);

		bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surfaceKhr);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates,
	                             VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		ASSERT(false); // failed to find supported format!
		return VK_FORMAT_UNDEFINED;
	}

	VkFormat findDepthFormat(VkPhysicalDevice physicalDevice)
	{
		return findSupportedFormat(physicalDevice,
		                           {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		                           VK_IMAGE_TILING_OPTIMAL,
		                           VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
				VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, HWND windowHandle)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			RECT rect;
			if (GetWindowRect(windowHandle, &rect))
			{
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;
			}

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};
			auto max = [](uint32_t a, uint32_t b) { return a > b ? a : b; };
			auto min = [](uint32_t a, uint32_t b) { return a < b ? a : b; };
			actualExtent.width = max(capabilities.minImageExtent.width,
			                         min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = max(capabilities.minImageExtent.height,
			                          min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	uint32_t findMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		ASSERT(false); // failed to find suitable memory type!
		return UINT32_MAX;
	}

	std::string ParseVkResult(VkResult res)
	{
		switch (res)
		{
		case VK_SUCCESS: { return "VK_SUCCESS"; }
		case VK_NOT_READY: { return "VK_NOT_READY"; }
		case VK_TIMEOUT: { return "VK_TIMEOUT"; }
		case VK_EVENT_SET: { return "VK_EVENT_SET"; }
		case VK_EVENT_RESET: { return "VK_EVENT_RESET"; }
		case VK_INCOMPLETE: { return "VK_INCOMPLETE"; }
		case VK_ERROR_OUT_OF_HOST_MEMORY: { return "VK_ERROR_OUT_OF_HOST_MEMORY"; }
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: { return "VK_ERROR_OUT_OF_DEVICE_MEMORY"; }
		case VK_ERROR_INITIALIZATION_FAILED: { return "VK_ERROR_INITIALIZATION_FAILED"; }
		case VK_ERROR_DEVICE_LOST: { return "VK_ERROR_DEVICE_LOST"; }
		case VK_ERROR_MEMORY_MAP_FAILED: { return "VK_ERROR_MEMORY_MAP_FAILED"; }
		case VK_ERROR_LAYER_NOT_PRESENT: { return "VK_ERROR_LAYER_NOT_PRESENT"; }
		case VK_ERROR_EXTENSION_NOT_PRESENT: { return "VK_ERROR_EXTENSION_NOT_PRESENT"; }
		case VK_ERROR_FEATURE_NOT_PRESENT: { return "VK_ERROR_FEATURE_NOT_PRESENT"; }
		case VK_ERROR_INCOMPATIBLE_DRIVER: { return "VK_ERROR_INCOMPATIBLE_DRIVER"; }
		case VK_ERROR_TOO_MANY_OBJECTS: { return "VK_ERROR_TOO_MANY_OBJECTS"; }
		case VK_ERROR_FORMAT_NOT_SUPPORTED: { return "VK_ERROR_FORMAT_NOT_SUPPORTED"; }
		case VK_ERROR_FRAGMENTED_POOL: { return "VK_ERROR_FRAGMENTED_POOL"; }
		case VK_ERROR_UNKNOWN: { return "VK_ERROR_UNKNOWN"; }
		case VK_ERROR_OUT_OF_POOL_MEMORY: { return "VK_ERROR_OUT_OF_POOL_MEMORY"; }
		case VK_ERROR_INVALID_EXTERNAL_HANDLE: { return "VK_ERROR_INVALID_EXTERNAL_HANDLE"; }
		case VK_ERROR_FRAGMENTATION: { return "VK_ERROR_FRAGMENTATION"; }
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: { return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS"; }
		case VK_ERROR_SURFACE_LOST_KHR: { return "VK_ERROR_SURFACE_LOST_KHR"; }
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: { return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"; }
		case VK_SUBOPTIMAL_KHR: { return "VK_SUBOPTIMAL_KHR"; }
		case VK_ERROR_OUT_OF_DATE_KHR: { return "VK_ERROR_OUT_OF_DATE_KHR"; }
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: { return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR"; }
		case VK_ERROR_VALIDATION_FAILED_EXT: { return "VK_ERROR_VALIDATION_FAILED_EXT"; }
		case VK_ERROR_INVALID_SHADER_NV: { return "VK_ERROR_INVALID_SHADER_NV"; }
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			{
				return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
			}
		case VK_ERROR_NOT_PERMITTED_EXT: { return "VK_ERROR_NOT_PERMITTED_EXT"; }
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: { return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT"; }
		case VK_THREAD_IDLE_KHR: { return "VK_THREAD_IDLE_KHR"; }
		case VK_THREAD_DONE_KHR: { return "VK_THREAD_DONE_KHR"; }
		case VK_OPERATION_DEFERRED_KHR: { return "VK_OPERATION_DEFERRED_KHR"; }
		case VK_OPERATION_NOT_DEFERRED_KHR: { return "VK_OPERATION_NOT_DEFERRED_KHR"; }
		case VK_PIPELINE_COMPILE_REQUIRED_EXT: { return "VK_PIPELINE_COMPILE_REQUIRED_EXT"; }
		default:
			ASSERT(false);
		}
		return "";
	}
}
