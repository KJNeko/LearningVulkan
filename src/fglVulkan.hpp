#ifndef fglVulakn
#define fglVulakn
#include <string>
#include <vector>
#include <ranges>
#include <algorithm>
#include <iterator>



#include <vulkan/vulkan_raii.hpp>

namespace fgl::vulkan
{

	struct AppInfo
	{
		const char* appName;
		uint32_t appVersion;
		const char* engineName;
		uint32_t engineVersion;
		uint32_t apiVersion;
		std::vector<const char*> layer{};
		std::vector<const char*> extentions{};
		uint32_t queue_count{};
		float queue_priority{};
	};



	/*
	Buffer I/O

	?????????set?
	binding
	size
	memory flags

	physical_device
	Device
	*/

	class Vulkan;

	class Buffer
	{
	public:

		vk::raii::Buffer buffer;
		uint32_t binding;

		Buffer() = delete;

		Buffer(
			const Vulkan& vulkan,
			const vk::DeviceSize& size,
			const vk::BufferUsageFlagBits usageflags,
			const vk::SharingMode sharingmode,
			const uint32_t _binding )
			:
			buffer( create_buffer( vulkan, size, usageflags, sharingmode ) ),
			binding( _binding )
		{}

		Buffer( Buffer&& ) = default;

		vk::raii::Buffer create_buffer(
			const Vulkan& vulkan,
			const vk::DeviceSize size,
			const vk::BufferUsageFlagBits usageflags,
			const vk::SharingMode sharingmode );

	};


	class Memory
	{

		vk::raii::DeviceMemory m_memory;
		std::vector<size_t> bufferoffsets{};

	public:

		Memory() = delete;

		Memory(
			const Vulkan& vulkan,
			const std::vector<Buffer>& buffs,
			const vk::MemoryPropertyFlags& flags )
			:
			m_memory( create_device_memory( vulkan, buffs, flags ) )
		{}

		std::pair<uint32_t, vk::DeviceSize> get_memory_type(
			const vk::MemoryPropertyFlags& flags,
			const vk::PhysicalDeviceMemoryProperties& device_memory_properties );

		vk::raii::DeviceMemory create_device_memory(
			const Vulkan& vulkan,
			const std::vector<Buffer>& buffs,
			const vk::MemoryPropertyFlags& flags );
	};


	//vk::context vk::instance
	class Vulkan
	{
	public:
		const vk::raii::Context context {};
		const vk::raii::Instance instance;
		const vk::raii::PhysicalDevice physical_device;
		const uint32_t queue_family_index;
		const vk::raii::Device device;




		uint32_t index_of_first_queue_family( const vk::QueueFlagBits flag );

		vk::raii::Instance create_instance( const AppInfo& info );

		vk::raii::Device create_device( uint32_t queue_count, float queue_priority );

		Vulkan( const AppInfo& info )
			:
			instance( create_instance( info ) ),
			physical_device( std::move( vk::raii::PhysicalDevices( instance ).front() ) ),
			queue_family_index( index_of_first_queue_family( vk::QueueFlagBits::eCompute ) ),
			device( create_device( info.queue_count, info.queue_priority ) )
		{}

	};

}


#endif /* fglVulakn */
