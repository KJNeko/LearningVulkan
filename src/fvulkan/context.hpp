#ifndef FGL_VULKAN_CONTEXT_HPP_INCLUDED
#define FGL_VULKAN_CONTEXT_HPP_INCLUDED

#include <cstdint>
#include <vector>

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




        uint32_t index_of_first_queue_family( const vk::QueueFlagBits flag ) const;

        vk::raii::Instance create_instance( const AppInfo& info );

        vk::raii::Device create_device( uint32_t queue_count, float queue_priority );

        Context( const AppInfo& info )
            :
            instance( create_instance( info ) ),
            physical_device( std::move( vk::raii::PhysicalDevices( instance ).front() ) ),
            queue_family_index( index_of_first_queue_family( vk::QueueFlagBits::eCompute ) ),
            device( create_device( info.queue_count, info.queue_priority ) )
        {}

    };



}

#endif /* FGL_VULKAN_CONTEXT_HPP_INCLUDED */