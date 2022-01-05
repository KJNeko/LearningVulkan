

#include "fglVulkan.hpp"


using namespace fgl::vulkan;


//BUFFER
vk::raii::Buffer Buffer::create_buffer(
    Vulkan& vulkan,
    vk::DeviceSize& size,
    vk::BufferUsageFlagBits& usageflags,
    vk::SharingMode& sharingmode )
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
std::pair<uint32_t, vk::DeviceSize> Memory::get_memory_type(
    const vk::MemoryPropertyFlags& flags,
    const vk::PhysicalDeviceMemoryProperties& device_memory_properties )
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

vk::raii::DeviceMemory Memory::create_device_memory(
    const Vulkan& vulkan,
    const std::vector<Buffer>& buffs,
    const vk::MemoryPropertyFlags& flags )
{
    size_t accum { 0 };

    const auto [memindex, size] { get_memory_type( flags, vulkan.physical_device.getMemoryProperties() ) };

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
    return vulkan.device.allocateMemory( memInfo );
}


//VULKAN
uint32_t Vulkan::index_of_first_queue_family( const vk::QueueFlagBits flag )
{
    const auto has_flag {
        [flag]( const vk::QueueFamilyProperties& qfp ) noexcept -> bool
        { // lambda function for find_if predicate
            return static_cast< bool >( qfp.queueFlags & flag );
        }
    };

    const auto end = physical_device.getQueueFamilyProperties().cend();
    const std::vector<vk::QueueFamilyProperties>& props = physical_device.getQueueFamilyProperties();

    if( const auto it { std::ranges::find_if( props, has_flag ) };
        it != end )
    {
        return static_cast< uint32_t >( std::distance( props.cbegin(), it ) );
    }
    else throw std::runtime_error(
        "Vulkan couldn't find a graphics queue family."
    );
}

vk::raii::Instance Vulkan::create_instance( const AppInfo& info )
{
    const vk::ApplicationInfo appInfo( info.appName, info.appVersion, info.engineName, info.engineVersion, info.apiVersion );
    const vk::InstanceCreateInfo ci(
        vk::InstanceCreateFlags {}, &appInfo,
        static_cast< uint32_t >( info.layer.size() ), info.layer.data(),
        static_cast< uint32_t >( info.extentions.size() ), info.extentions.data()
    );
    return vk::raii::Instance( context, ci );
}

vk::raii::Device Vulkan::create_device( uint32_t queue_count, float queue_priority )
{
    const vk::DeviceQueueCreateInfo device_queue_ci(
        {}, queue_family_index, queue_count, &queue_priority
    );
    const vk::DeviceCreateInfo device_ci( {}, device_queue_ci );

    return vk::raii::Device( physical_device, device_ci );
}
