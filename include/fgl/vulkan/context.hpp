#ifndef FGL_VULKAN_CONTEXT_HPP_INCLUDED
#define FGL_VULKAN_CONTEXT_HPP_INCLUDED

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include <iostream>


namespace fgl::vulkan
{

	struct AppInfo
	{
		uint32_t apiVersion;
		std::vector<const char*> layer {};
		std::vector<const char*> extentions {};
		uint32_t queue_count {};
		float queue_priority {};
	};

	//vk::context vk::instance
	class Context
	{
	public:
		const vk::raii::Context context {};
		const vk::raii::Instance instance;
		const vk::raii::PhysicalDevice physical_device;
		const uint32_t queue_family_index;
		const vk::raii::Device device;

		uint32_t index_of_first_queue_family(const vk::QueueFlagBits flag ) const;

		vk::PhysicalDeviceProperties properties;

		[[nodiscard]] explicit
		Context( const AppInfo& info, const bool debug_printing = false );

	};

}

#endif /* FGL_VULKAN_CONTEXT_HPP_INCLUDED */
