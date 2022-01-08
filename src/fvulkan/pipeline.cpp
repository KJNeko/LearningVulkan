

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



	vk::raii::PipelineLayout Pipeline::create_pipeline_layout( const Context& cntx )
	{
		const vk::PipelineLayoutCreateInfo ci( vk::PipelineLayoutCreateFlags(), *descriptor_set_layouts );

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

	vk::raii::DescriptorSets Pipeline::create_descriptor_sets( const Context& cntx )
	{
		vk::DescriptorSetAllocateInfo alloc_info( *pool, *descriptor_set_layouts );
		return vk::raii::DescriptorSets( cntx.device, alloc_info );
	}



}


