#include <vulkan/vulkan_raii.hpp>

#include <fgl/vulkan/memory.hpp>
#include <fgl/vulkan/context.hpp>

namespace fgl::vulkan
{
	size_t Buffer::bytecount { 0 }; //Static member of Buffer
	//BUFFER
	vk::raii::Buffer Buffer::create_buffer(
		const Context& vulkan,
		const vk::DeviceSize size,
		const vk::BufferUsageFlagBits usageflags,
		const vk::SharingMode sharingmode ) const
	{
		constexpr uint32_t number_of_family_indexes { 1 };

		const auto index { vulkan.index_of_first_queue_family( vk::QueueFlagBits::eCompute ) };

		const vk::BufferCreateInfo ci(
			{},
			size,
			usageflags,
			sharingmode,
			number_of_family_indexes,
			&index
		);
		return vulkan.device.createBuffer( ci );
	}

	//MEMORY
	std::pair<uint32_t, vk::DeviceSize> Buffer::get_memory_type(
		const vk::PhysicalDeviceMemoryProperties& device_memory_properties,
		const vk::MemoryPropertyFlags flags ) const
	{
		for(
			uint32_t current_memory_index = 0;
			current_memory_index < device_memory_properties.memoryTypeCount;
			++current_memory_index )
		{
			const vk::MemoryType memoryType {
				device_memory_properties.memoryTypes[current_memory_index]
			};
			if( flags & memoryType.propertyFlags )
			{
				const vk::DeviceSize memory_heap_size {
					 device_memory_properties.memoryHeaps[memoryType.heapIndex].size
				};
				return std::pair { current_memory_index, memory_heap_size };
			}
		}
		throw std::runtime_error( "Failed to get memory type." );
	}

	vk::raii::DeviceMemory Buffer::create_device_memory(
		const Context& context,
		const vk::MemoryPropertyFlags flags ) const
	{
		const auto [memindex, size] { get_memory_type( context.physical_device.getMemoryProperties(), flags ) };

		bytecount += buffer.getMemoryRequirements().size;
		if( context.physical_device.getMemoryProperties().memoryHeaps[1].size < bytecount )
		{
			std::stringstream ss;
			ss << "Attempting to allocate too much memory (in Byte)\n";
			ss << "\tMemory allocated: " << bytesize << "\n";
			ss << "\tMaximum Memory: " << context.physical_device.getMemoryProperties().memoryHeaps[1].size << "\n";
			ss << "\tMemory avalilable: " << context.properties.limits.maxMemoryAllocationCount - ( bytecount - bytesize ) << "\n";
			ss << "\tMemory Over: " << bytecount - context.physical_device.getMemoryProperties().memoryHeaps[1].size - ( bytecount - bytesize ) << "\n";

			throw std::runtime_error( ss.str() );
		}

		const vk::MemoryAllocateInfo memInfo( buffer.getMemoryRequirements().size, memindex );

		return context.device.allocateMemory( memInfo );
	}

	void* Buffer::get_memory() const
	{
		return memory.mapMemory( 0, bytesize );
	}

}
