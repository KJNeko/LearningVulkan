#ifndef FGL_VULKAN_MEMORY_HPP_INCLUDED
#define FGL_VULKAN_MEMORY_HPP_INCLUDED

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "context.hpp"

namespace fgl::vulkan
{

    class Buffer
    {
    public:

        uint32_t binding;
        vk::raii::Buffer buffer;

        Buffer() = delete;

        Buffer(
            const Context& context,
            const vk::DeviceSize& size,
            const vk::BufferUsageFlagBits usageflags,
            const vk::SharingMode sharingmode,
            const uint32_t _binding )
            :
            binding( _binding ),
            buffer( create_buffer( context, size, usageflags, sharingmode ) )
        {}

        Buffer( Buffer&& ) = default;

        vk::raii::Buffer create_buffer(
            const Context& context,
            const vk::DeviceSize size,
            const vk::BufferUsageFlagBits usageflags,
            const vk::SharingMode sharingmode );

    };



    class Memory
    {

        vk::raii::DeviceMemory m_memory;
        std::vector<size_t> bufferoffsets {};
        vk::MemoryPropertyFlags flags;

    public:

        Memory() = delete;

        template <typename T>
        Memory(
            const Context& context,
            const T& buffs,
            const vk::MemoryPropertyFlags& _flags )
            :
            m_memory( create_device_memory( context, buffs, _flags ) ),
            flags( _flags )
        {}

        std::pair<uint32_t, vk::DeviceSize> get_memory_type(
            const vk::MemoryPropertyFlags& flags,
            const vk::PhysicalDeviceMemoryProperties& device_memory_properties );

        template <typename T>
        vk::raii::DeviceMemory create_device_memory(
            const Context& context,
            const T& buffs,
            const vk::MemoryPropertyFlags& flags )
        {
            size_t accum { 0 };

            const auto [memindex, size] { get_memory_type( flags, context.physical_device.getMemoryProperties() ) };

            for( auto& buff : buffs )
            {
                bufferoffsets.push_back( accum );
                accum += buff.buffer.getMemoryRequirements().size;
                if( accum >= size )
                {
                    throw std::runtime_error( "Allocating too much memory" );
                }
            }
            const vk::MemoryAllocateInfo memInfo( accum, memindex );
            return context.device.allocateMemory( memInfo );
        }

    };

}

#endif /* FGL_VULKAN_MEMORY_HPP_INCLUDED */
