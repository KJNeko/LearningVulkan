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
		vk::raii::DescriptorPool pool;
		vk::raii::PipelineLayout layout;
		vk::raii::Pipeline pipeline;
		vk::raii::DescriptorSets sets;

		Pipeline() = delete;

		/// TODO constrain template to range of buffers
		template <typename T>
		Pipeline(
			const Context& cntx,
			const std::filesystem::path& shaderpath,
			const std::string& shader_init_name,
			const T& buffers )
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

		vk::raii::ShaderModule create_shader_module(
			const Context& cntx,
			const std::filesystem::path path
		) const;

		/// TODO constrain template to range of buffers
		template <typename T>
		vk::raii::DescriptorSetLayout create_descriptor_set_layout(
			const Context& cntx,
			const T& buffers )
		{
			std::vector<vk::DescriptorSetLayoutBinding> setBindings;
			for( const auto& buffer : buffers )
			{
				constexpr uint32_t descriptor_count{ 1 };
				setBindings.emplace_back(
					buffer.binding,
					buffer.buffer_type,
					descriptor_count,
					vk::ShaderStageFlagBits::eCompute
				);
			}
			const vk::DescriptorSetLayoutCreateInfo ci(
				{},
				static_cast<uint32_t>(setBindings.size()),
				setBindings.data()
			);
			return vk::raii::DescriptorSetLayout( cntx.device, ci );
		}

		/// TODO constrain template to range of buffers
		template <typename T>
		vk::raii::DescriptorPool create_descriptor_pool(
			const Context& cntx,
			const T& buffers ) const
		{
			const auto count = static_cast< uint32_t >( buffers.size() );
			const vk::DescriptorPoolSize poolsize { buffers.front().buffer_type, count };
			constexpr uint32_t max_sets{ 1 };
			constexpr uint32_t pool_size_count{ 1 };
			const vk::DescriptorPoolCreateInfo ci(
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				max_sets,
				pool_size_count,
				&poolsize
			);
			return vk::raii::DescriptorPool( cntx.device, ci );
		}

		vk::raii::PipelineLayout create_pipeline_layout( const Context& cntx ) const;

		vk::raii::Pipeline create_pipeline(
			const Context& cntx,
			const std::string init_function_name
		) const;

		vk::raii::DescriptorSets create_descriptor_sets( const Context& cntx ) const;
	};



}




#endif /* FGL_VULKAN_PIPELINE_HPP_INCLUDE */
