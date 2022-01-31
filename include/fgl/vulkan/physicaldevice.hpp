#ifndef FGL_VULKAN_PHYSICALDEVICE_HPP_INCLUDE
#define FGL_VULKAN_PHYSICALDEVICE_HPP_INCLUDE

#include "instance.hpp"

#include <algorithm>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>

namespace fgl::vulkan
{

class PhysicalDevice
{
	VkPhysicalDevice physicalDevice { nullptr };

	PhysicalDevice( PhysicalDevice const& rhs ) = delete;
	PhysicalDevice& operator=( PhysicalDevice const& rhs ) = delete;

	bool
	deviceHasFlag( const VkPhysicalDevice& device, const VkQueueFlagBits flag ) const
	{
		const auto has_flag {
			[ flag ]( const VkQueueFamilyProperties& qfp ) noexcept -> bool {
				return static_cast<bool>(
					qfp.queueFlags & static_cast<uint>( flag ) );
			} };

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(
			device, &queueFamilyCount, nullptr );

		std::vector<VkQueueFamilyProperties> props( queueFamilyCount );
		vkGetPhysicalDeviceQueueFamilyProperties(
			device, &queueFamilyCount, props.data() );

		return std::ranges::find_if( props, has_flag ) != props.cend();
	}

  public:
	operator VkPhysicalDevice&()
	{
		return physicalDevice;
	}

	PhysicalDevice() = delete;

	PhysicalDevice( fgl::vulkan::Instance& instance, VkQueueFlagBits flagbits )
	{
		uint32_t deviceCount { 0 };
		vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr );

		if ( deviceCount == 0 )
		{
			throw std::runtime_error( "Failed to find GPUs with Vulkan Support" );
		}

		std::vector<VkPhysicalDevice> devices( deviceCount );
		vkEnumeratePhysicalDevices( instance, &deviceCount, devices.data() );

		for ( const auto& device : devices )
		{
			if ( deviceHasFlag( device, flagbits ) )
			{
				physicalDevice = device;
				return;
			}
		}

		throw std::runtime_error( "Failed to find a device with required flags" );
	}
};

} // namespace fgl::vulkan

#endif /* FGL_VULKAN_PHYSICALDEVICE_HPP_INCLUDE */
