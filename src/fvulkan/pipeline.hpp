#ifndef FGL_VULKAN_PIPELINE_HPP_INCLUDE
#define FGL_VULKAN_PIPELINE_HPP_INCLUDE

#include <vulkan/vulkan_raii.hpp>
#include "context.hpp"
#include "memory.hpp"

#include <string>
#include <filesystem>
#include <vector>

namespace fgl::vulkan
{

    //TODO PIPELINE
    /*
    DEPENDS


    vk::raii::DescriptorSetLayout
    vk::raii::PipelineLayout
    vk::raii::Pipeline


    Needs path to shader.
    shader "main" name


    DescriptorsetLayout needs buffers and types. Can be gotten from class Memory in memory.hpp
    */




    class Pipeline
    {
    public:

        vk::raii::ShaderModule shader_module;
        std::vector<vk::DescriptorPoolSize> poolsizes {};
        std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layout;


        vk::raii::DescriptorPool pool;

        vk::raii::PipelineLayout layout;
        vk::raii::Pipeline pipeline;

        Pipeline() = delete;

        template <typename T>
        Pipeline( const Context& cntx, const std::filesystem::path& shaderpath, const std::string& shader_init_name, const T& buffers )
            :
            shader_module( create_shader_module( cntx, shaderpath ) ),
            descriptor_set_layout( create_descriptor_set_layout( cntx, buffers ) ),
            pool( create_descriptor_pool( cntx ) ),
            layout( create_pipeline_layout( cntx ) ),
            pipeline( create_pipeline( cntx, shader_init_name ) )
        {}

        vk::raii::ShaderModule create_shader_module( const Context& cntx, const std::filesystem::path path );

        template <typename T>
        std::vector<vk::raii::DescriptorSetLayout> create_descriptor_set_layout( const Context& cntx, const T& buffers )
        {
            std::vector<std::vector<vk::DescriptorSetLayoutBinding>> bindings;
            std::vector<vk::DescriptorType> type;
            for( const auto& buffer : buffers )
            {
                if( buffer.set + 1 >= bindings.size() || buffer.set + 1 >= type.size() )
                {
                    bindings.resize( buffer.set + 1 );
                    type.resize( buffer.set + 1 );
                }

                bindings.at( buffer.set ).push_back( vk::DescriptorSetLayoutBinding( buffer.binding, buffer.buffer_type, 1, vk::ShaderStageFlagBits::eCompute ) );
                type.at( buffer.set ) = buffer.buffer_type;
            }

            std::vector<vk::raii::DescriptorSetLayout> layouts;
            for( const auto& binding : bindings )
            {
                const vk::DescriptorSetLayoutCreateInfo ci
                (
                    {},
                    static_cast< uint32_t >( binding.size() ),
                    binding.data()
                );

                layouts.push_back( vk::raii::DescriptorSetLayout( cntx.device, ci ) );
            }

            assert( bindings.size() == type.size() );

            for( size_t i = 0; i < type.size(); ++i )
            {
                poolsizes.push_back( vk::DescriptorPoolSize( type[i], static_cast< uint32_t >( bindings[i].size() ) ) );
            }

            return layouts;
        }

        vk::raii::DescriptorPool create_descriptor_pool( const Context& cntx );

        vk::raii::PipelineLayout create_pipeline_layout( const Context& cntx );

        vk::raii::Pipeline create_pipeline( const Context& cntx, const std::string init_function_name );





    };



}




#endif /* FGL_VULKAN_PIPELINE_HPP_INCLUDE */
