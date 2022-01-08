#ifndef FGL_VULKAN_PIPELINE_HPP_INCLUDE
#define FGL_VULKAN_PIPELINE_HPP_INCLUDE

#include <vulkan/vulkan_raii.hpp>
#include "context.hpp"
#include "memory.hpp"

#include <string>
#include <filesystem>
#include <vector>

#include <iostream>

namespace fgl::vulkan
{

	class Pipeline
	{
	public:

		vk::raii::ShaderModule shader_module;
		vk::raii::DescriptorSetLayout descriptor_set_layouts;

		//std::vector<vk::DescriptorSetLayoutBinding> setBindings;

		vk::raii::DescriptorPool pool;

		vk::raii::PipelineLayout layout;
		vk::raii::Pipeline pipeline;

		vk::raii::DescriptorSets sets;

		Pipeline() = delete;

		template <typename T>
		Pipeline( const Context& cntx, const std::filesystem::path& shaderpath, const std::string& shader_init_name, const T& buffers )
			:
			shader_module( create_shader_module( cntx, shaderpath ) ),
			descriptor_set_layouts( create_descriptor_set_layout( cntx, buffers ) ),
			pool( create_descriptor_pool( cntx, buffers ) ),
			layout( create_pipeline_layout( cntx ) ),
			pipeline( create_pipeline( cntx, shader_init_name ) ),
			sets( create_descriptor_sets( cntx ) )
		{
			std::vector<vk::WriteDescriptorSet> writeset;
			std::vector<vk::DescriptorBufferInfo> bufferinfo;

			writeset.resize( buffers.size() );
			bufferinfo.resize( buffers.size() );

			for( size_t i = 0; i < buffers.size(); ++i )
			{
				constexpr vk::DescriptorImageInfo* imgpointer { nullptr };
				bufferinfo.at( i ) = vk::DescriptorBufferInfo( *buffers.at( i ).buffer, 0, buffers.at( i ).bytesize );
				writeset.at( i ) = vk::WriteDescriptorSet( *sets.front(), buffers.at( i ).binding, 0, 1, buffers.at( i ).buffer_type, imgpointer, &bufferinfo.at( i ) );
			}

			cntx.device.updateDescriptorSets( writeset, nullptr );
			std::cout << "Pipeline made successfuly with " << buffers.size() << " buffers." << std::endl;
		}

		vk::raii::ShaderModule create_shader_module( const Context& cntx, const std::filesystem::path path );

		template <typename T>
		vk::raii::DescriptorSetLayout create_descriptor_set_layout( const Context& cntx, const T& buffers )
		{
			//setBindings.resize( buffers.size() + 1 );
			std::vector<vk::DescriptorSetLayoutBinding> setBindings;
			for( const auto& buffer : buffers )
			{
				vk::DescriptorSetLayoutBinding binding( buffer.binding, buffer.buffer_type, 1, vk::ShaderStageFlagBits::eCompute );
				setBindings.push_back( binding );
			}

			vk::DescriptorSetLayoutCreateInfo ci( {}, setBindings.size(), setBindings.data() );
			return vk::raii::DescriptorSetLayout( cntx.device, ci );
		}
		template <typename T>
		vk::raii::DescriptorPool create_descriptor_pool( const Context& cntx, const T& buffers )
		{
			auto count = static_cast< uint32_t >( buffers.size() );
			auto& buffer = buffers.front();
			vk::DescriptorPoolSize poolsize { buffer.buffer_type, count };

			vk::DescriptorPoolCreateInfo ci( vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, 1, &poolsize );
			return vk::raii::DescriptorPool( cntx.device, ci );
		}

		vk::raii::PipelineLayout create_pipeline_layout( const Context& cntx );

		vk::raii::Pipeline create_pipeline( const Context& cntx, const std::string init_function_name );

		vk::raii::DescriptorSets create_descriptor_sets( const Context& cntx );



	};



}




#endif /* FGL_VULKAN_PIPELINE_HPP_INCLUDE */
