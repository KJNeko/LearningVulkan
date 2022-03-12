#ifndef FGL_VULKAN_DEVICE_HPP_INCLUDED
#define FGL_VULKAN_DEVICE_HPP_INCLUDED

#include <algorithm>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>

#include "./fgl/vulkan/queue.hpp"
#include "./fgl/vulkan/swapchain.hpp"
#include "./fgl/vulkan/window.hpp"
namespace fgl::vulkan
{

class Device
{
	VkDevice device { nullptr };

	fgl::vulkan::Queue deviceQueue;
	fgl::vulkan::Queue presentQueue;

  public:
	fgl::vulkan::Swapchain swapChain;

	Device() = delete;

	Device( Device const& rhs ) = delete;
	Device& operator=( Device const& rhs ) = delete;

  private:
	template <std::ranges::forward_range T>
	requires std::same_as<const char*, std::ranges::range_value_t<T>>
		VkDevice createDevice(
			const VkPhysicalDevice& deviceRef,
			const VkQueueFlagBits flags,
			T& deviceExtensions,
			float queuePriority )
	{
		VkDeviceQueueCreateInfo queueCreateInfo {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex =
			fgl::vulkan::internal::findIndexForFlags( deviceRef, flags );
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures {};

		VkDeviceCreateInfo createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount =
			static_cast<uint32_t>( deviceExtensions.size() );
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// TODO Debug stuffs to do here
		/*if (enableValidationLayers) {
					createInfo.enabledLayerCount =
		   static_cast<uint32_t>(validationLayers.size());
					createInfo.ppEnabledLayerNames = validationLayers.data();
				} else {
					createInfo.enabledLayerCount = 0;
				}
		*/

		createInfo.enabledLayerCount = 0;

		VkDevice tempdevice;

		if ( vkCreateDevice( deviceRef, &createInfo, nullptr, &tempdevice ) !=
			 VK_SUCCESS )
		{
			throw std::runtime_error( "Failed to create logical device!" );
		}

		return tempdevice;
	}


  public :

	  operator VkDevice&()
	{
		return device;
	}

	VkQueue& getQueue()
	{
		return deviceQueue;
	}

	VkQueue& getPresentQueue()
	{
		return presentQueue;
	}

	// Formatter gets a little confused with the requires statement
	// clang-format off
	template <std::ranges::forward_range T>
	requires std::same_as<const char*, std::ranges::range_value_t<T>>
	Device(
		fgl::vulkan::PhysicalDevice& physicalDevice,
		VkQueueFlagBits flags,
		T& deviceExtensions,
		fgl::vulkan::Window& window,
		float queuePriority = 0.0f )
		:
		device( createDevice(physicalDevice, flags, deviceExtensions, queuePriority ) ),
		deviceQueue( device, physicalDevice, flags ),
		presentQueue( device, physicalDevice, flags, true, window.surface ),
		swapChain( device, physicalDevice, window)
	{}
	// clang-format on

	~Device()
	{
		vkDeviceWaitIdle( *this );
	}
};

} // namespace fgl::vulkan

#endif /* FGL_VULKAN_DEVICE_HPP_INCLUDED */
