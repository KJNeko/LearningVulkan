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
        uint32_t binding;
        vk::DeviceSize bytesize;
        vk::raii::Buffer buffer;
        vk::raii::DeviceMemory memory;
        vk::DescriptorType buffer_type;

        Buffer() = delete;

        Buffer(
            const Context& context,
            const vk::DeviceSize& size,
            const vk::BufferUsageFlagBits usageflags,
            const vk::SharingMode sharingmode,
            const uint32_t _binding,
            const vk::MemoryPropertyFlags flags,
            const vk::DescriptorType type )
            :
            binding( _binding ),
            bytesize( size ),
            buffer( create_buffer( context, size, usageflags, sharingmode ) ),
            memory( create_device_memory( context, flags ) ),
            buffer_type( type )
        {
            buffer.bindMemory( *memory, 0 );
            std::cout << "Memory created successfully with binding:" << _binding << std::endl;
        }

        std::pair<uint32_t, vk::DeviceSize> get_memory_type(
            const vk::PhysicalDeviceMemoryProperties& device_memory_properties,
            const vk::MemoryPropertyFlags flags );

        vk::raii::Buffer create_buffer(
            const Context& context,
            const vk::DeviceSize size,
            const vk::BufferUsageFlagBits usageflags,
            const vk::SharingMode sharingmode );

        vk::raii::DeviceMemory create_device_memory(
            const Context& context,
            const vk::MemoryPropertyFlags flags )
        {
            const auto [memindex, size] { get_memory_type( context.physical_device.getMemoryProperties(), flags ) };

            const vk::MemoryAllocateInfo memInfo( bytesize, memindex );

            return context.device.allocateMemory( memInfo );
        }

        void* get_memory()
        {
            return memory.mapMemory( 0, bytesize );
        }

    };

}

#endif /* FGL_VULKAN_MEMORY_HPP_INCLUDED */
