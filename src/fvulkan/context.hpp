#ifndef FGL_VULKAN_CONTEXT_HPP_INCLUDED
#define FGL_VULKAN_CONTEXT_HPP_INCLUDED

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include <iostream>


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

    //vk::context vk::instance
    class Context
    {
    public:
        const vk::raii::Context context {};
        const vk::raii::Instance instance;
        const vk::raii::PhysicalDevice physical_device;
        const uint32_t queue_family_index;
        const vk::raii::Device device;


        void verify_version_requirements();

        uint32_t index_of_first_queue_family( const vk::QueueFlagBits flag ) const;

        vk::raii::Instance create_instance( const AppInfo& info );

        vk::raii::Device create_device( uint32_t queue_count, float queue_priority );

        Context( const AppInfo& info, bool debug_printing = false )
            :
            instance( create_instance( info ) ),
            physical_device( std::move( vk::raii::PhysicalDevices( instance ).front() ) ),
            queue_family_index( index_of_first_queue_family( vk::QueueFlagBits::eCompute ) ),
            device( create_device( info.queue_count, info.queue_priority ) )
        {
            const uint32_t loaded_version { context.enumerateInstanceVersion() };
            const uint32_t loader_major { VK_VERSION_MAJOR( loaded_version ) };
            const uint32_t loader_minor { VK_VERSION_MINOR( loaded_version ) };
            const uint32_t target_major { VK_VERSION_MAJOR( info.apiVersion ) };
            const uint32_t target_minor { VK_VERSION_MINOR( info.apiVersion ) };

            if( loader_major < target_major
                || ( loader_major == target_major && loader_minor < target_minor ) )
            {
                std::stringstream ss;
                ss
                    << "Vulkan API " << target_major << '.' << target_minor
                    << " is not supported (system has version: "
                    << loader_major << '.' << loader_minor << ')';
                throw std::runtime_error( ss.str() );
            }


            if( debug_printing ) [[unlikely]]
            {
                const uint32_t loaded_version { context.enumerateInstanceVersion() };
                const uint32_t loader_patch { VK_VERSION_PATCH( loaded_version ) };
                const uint32_t target_patch { VK_VERSION_PATCH( info.apiVersion ) };
                const auto& properties { physical_device.getProperties() };
                std::cout
                    << "\n\tDevice Name: " << properties.deviceName
                    << "\n\tMinimum required Vulkan API v"
                    << target_major << '.' << target_minor << '.' << target_patch
                    << "\n\tDetected running Vulkan API v"
                    << loader_major << '.' << loader_minor << '.' << loader_patch
                    << "\n\tHas support for  Vulkan API v"
                    << VK_VERSION_MAJOR( properties.apiVersion ) << '.'
                    << VK_VERSION_MINOR( properties.apiVersion ) << '.'
                    << VK_VERSION_PATCH( properties.apiVersion )
                    << "\n\tMax Compute Shared Memory Size: "
                    << properties.limits.maxComputeSharedMemorySize / 1024 << " KB"
                    << "\n\tCompute Queue Family Index: "
                    << queue_family_index
                    << "\n\tMax Compute Work Group Sizes: ";

                for( uint32_t index { 0 };
                    const auto n : properties.limits.maxComputeWorkGroupSize )
                {
                    std::cout << " [" << index++ << "]=" << n;
                }

                std::cout << "\n\tMax compute Work Group Count: ";
                for( uint32_t index { 0 };
                    const auto n : properties.limits.maxComputeWorkGroupCount )
                {
                    std::cout << " [" << index++ << "]=" << n;
                }

                std::cout << "\n\tMax Compute Inovactions: " << properties.limits.maxComputeWorkGroupInvocations << std::endl;

                std::cout << "\n" << std::endl;
            }
        }

    };



}

#endif /* FGL_VULKAN_CONTEXT_HPP_INCLUDED */