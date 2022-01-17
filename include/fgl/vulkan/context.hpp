#ifndef FGL_VULKAN_CONTEXT_HPP_INCLUDED
#define FGL_VULKAN_CONTEXT_HPP_INCLUDED

#include <cstdint>
#include <iostream>

#include <vulkan/vulkan_raii.hpp>

#include "./internal/version.hpp"


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

	class Context
	{
	public:
		const vk::raii::Context context;
		const vk::raii::Instance instance;
		const vk::raii::PhysicalDevice physical_device;
		const uint32_t queue_family_index;
		const vk::raii::Device device;
		const vk::PhysicalDeviceProperties properties;
		const internal::VersionInfo version_info;

		[[nodiscard]]
		uint32_t index_of_first_queue_family(const vk::QueueFlagBits flag ) const;

		[[nodiscard]] explicit Context( const AppInfo& info );

		void print_debug_info() const;
	};

}

#endif /* FGL_VULKAN_CONTEXT_HPP_INCLUDED */
