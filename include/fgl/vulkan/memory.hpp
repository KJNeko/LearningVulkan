#ifndef FGL_VULKAN_MEMORY_HPP_INCLUDED
#define FGL_VULKAN_MEMORY_HPP_INCLUDED

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "context.hpp"

#include <iostream>

namespace fgl::vulkan
{

	class Buffer
	{
	public:
		const uint32_t binding;
		const vk::DescriptorType buffer_type;
		const vk::DeviceSize bytesize;
		vk::raii::Buffer buffer;
		vk::raii::DeviceMemory memory;

		//Static memory
		static size_t bytecount;

		Buffer() = delete;
		Buffer( const Buffer& ) = delete;
		[[nodiscard]] explicit Buffer( Buffer&& ) = default;

		[[nodiscard]] explicit Buffer(
			const Context& context,
			const vk::DeviceSize& size,
			const vk::BufferUsageFlagBits usageflags,
			const vk::SharingMode sharingmode,
			const uint32_t binding_,
			const vk::MemoryPropertyFlags flags,
			const vk::DescriptorType type );

		void* get_memory() const;

		~Buffer()
		{
			bytecount -= bytesize;
		}
	};

}

#endif /* FGL_VULKAN_MEMORY_HPP_INCLUDED */
