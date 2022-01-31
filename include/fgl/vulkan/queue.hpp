#ifndef FGL_VULKAN_QUEUE_HPP_INCLUDED
#define FGL_VULKAN_QUEUE_HPP_INCLUDED

#include <bitset>

namespace fgl::vulkan
{

namespace internal
{
	uint32_t findIndexForFlags(
		VkPhysicalDevice deviceref,
		VkQueueFlagBits flags,
		bool presentSupportRequired = false,
		VkSurfaceKHR surface		= nullptr )
	{
		uint32_t queueFamilyCount { 0 };
		vkGetPhysicalDeviceQueueFamilyProperties(
			deviceref, &queueFamilyCount, nullptr );

		std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
		vkGetPhysicalDeviceQueueFamilyProperties(
			deviceref, &queueFamilyCount, queueFamilies.data() );

		uint32_t i { 0 };
		for ( const auto& queueFamily : queueFamilies )
		{

			if ( presentSupportRequired )
			{
				VkBool32 presentSupport = false;
				if ( vkGetPhysicalDeviceSurfaceSupportKHR(
						 deviceref, i, surface, &presentSupport ) != VK_SUCCESS )
				{
					throw std::runtime_error(
						"Failed to query the device Surface Support" );
				}

				if ( presentSupport )
				{
					return i;
				}
			}
			else
			{
				if ( queueFamily.queueFlags & static_cast<uint>( flags ) )
				{
					return i;
				}
			}
			++i;
		}

		throw std::runtime_error( "Failed to find a index for the required flags" );
	}
} // namespace internal

class Queue
{
	VkQueue queue;

	Queue() = delete;

	Queue( Queue const& rhs ) = delete;
	Queue& operator=( Queue const& rhs ) = delete;

  public:
	operator VkQueue&()
	{
		return queue;
	}

	Queue(
		VkDevice& deviceref,
		VkPhysicalDevice& physicalDevice,
		VkQueueFlagBits flags,
		bool presentSupportRequired = false,
		VkSurfaceKHR surface		= nullptr )
	{
		// Find the index for the bits we want
		uint32_t index = internal::findIndexForFlags(
			physicalDevice, flags, presentSupportRequired, surface );

		vkGetDeviceQueue( deviceref, index, 0, &queue );
	}
};

} // namespace fgl::vulkan

#endif /* FGL_VULKAN_QUEUE_HPP_INCLUDED */
