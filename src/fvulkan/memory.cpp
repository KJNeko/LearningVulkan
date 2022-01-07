#include <vulkan/vulkan_raii.hpp>

#include "memory.hpp"
#include "context.hpp"

namespace fgl::vulkan
{

    //BUFFER
    vk::raii::Buffer Buffer::create_buffer(
        const Context& vulkan,
        const vk::DeviceSize size,
        const vk::BufferUsageFlagBits usageflags,
        const vk::SharingMode sharingmode )
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
        const vk::MemoryPropertyFlags flags )
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



}