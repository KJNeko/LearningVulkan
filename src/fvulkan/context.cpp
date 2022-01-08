#include <vulkan/vulkan_raii.hpp>

#include "context.hpp"

namespace fgl::vulkan
{

    //VULKAN
    uint32_t Context::index_of_first_queue_family( const vk::QueueFlagBits flag ) const
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

    vk::raii::Instance Context::create_instance( const AppInfo& info )
    {
        const vk::ApplicationInfo appInfo( "VulkanCompute", 0, "ComputeEngine", 0, info.apiVersion );
        vk::InstanceCreateInfo ci(
            vk::InstanceCreateFlags {}, &appInfo,
            static_cast< uint32_t >( info.layer.size() ), info.layer.data(),
            static_cast< uint32_t >( info.extentions.size() ), info.extentions.data()
        );
        return vk::raii::Instance( context, ci );
    }

    vk::raii::Device Context::create_device( uint32_t queue_count, float queue_priority )
    {
        const vk::DeviceQueueCreateInfo device_queue_ci(
            {}, queue_family_index, queue_count, &queue_priority
        );

        std::vector<const char*> layers;
        std::vector<const char*> extentions { };

        const vk::DeviceCreateInfo device_ci( {}, device_queue_ci, layers, extentions );

        return vk::raii::Device( physical_device, device_ci );
    }

}