

#include "pipeline.hpp"

#include <vulkan/vulkan_raii.hpp>

#include "../fgl.hpp"

namespace fgl::vulkan
{
	vk::raii::ShaderModule Pipeline::create_shader_module( const Context& cntx, const std::filesystem::path path )
	{
		auto buff = fgl::misc::read_file( path );

		const vk::ShaderModuleCreateInfo ci(
			vk::ShaderModuleCreateFlags(),
			buff.size(),
			reinterpret_cast< const uint32_t* >( buff.data() )
		);

		return vk::raii::ShaderModule( cntx.device, ci );
	}

	vk::raii::DescriptorPool Pipeline::create_descriptor_pool( const Context& cntx )
	{
		vk::DescriptorPoolCreateInfo ci( vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, static_cast< uint32_t >( poolsizes.size() ), poolsizes );

		return vk::raii::DescriptorPool( cntx.device, ci );
	}

	vk::raii::PipelineLayout Pipeline::create_pipeline_layout( const Context& cntx )
	{

		std::vector<vk::DescriptorSetLayout> array;

		for( const auto& tlayout : descriptor_set_layout )
		{
			array.push_back( *tlayout );
		}

		const vk::PipelineLayoutCreateInfo ci( vk::PipelineLayoutCreateFlags(), array );

		//const vk::PipelineLayoutCreateInfo ci( {}, *descriptor_set_layout );
		return vk::raii::PipelineLayout( cntx.device, ci );
	}

	vk::raii::Pipeline Pipeline::create_pipeline( const Context& cntx, const std::string init_function_name )
	{
		const vk::PipelineShaderStageCreateInfo shader_stage_info(
			{},
			vk::ShaderStageFlagBits::eCompute,
			*shader_module,
			init_function_name.c_str()
		);

		const vk::ComputePipelineCreateInfo ci(
			{},
			shader_stage_info,
			*layout
		);

		return cntx.device.createComputePipeline(
			cntx.device.createPipelineCache( vk::PipelineCacheCreateInfo() ),
			ci
		);
	}



}


