
#include <cassert>
#include <cstdint> // uintptr_t

#include <fgl/vulkan/pipeline.hpp>

#include <vulkan/vulkan_raii.hpp>

#include <fgl/io.hpp>

namespace fgl::vulkan
{
	vk::raii::ShaderModule Pipeline::create_shader_module(
		const Context& cntx,
		const std::filesystem::path path ) const
	{
		const auto buff{ fgl::read_binary_file( path ) };

		/// TODO guarentee alignment
		// assert vector memory meets allignment requirements of uint32_t
		assert( reinterpret_cast< uintptr_t >( buff.data() ) % sizeof( uint32_t ) == 0 );

		const vk::ShaderModuleCreateInfo ci(
			vk::ShaderModuleCreateFlags(),
			buff.size(),
			reinterpret_cast< const uint32_t* >( // TODO ASSURE ALIGNMENT
				reinterpret_cast< const void* >( buff.data() )
			)
		);
		return vk::raii::ShaderModule( cntx.device, ci );
	}

	vk::raii::PipelineLayout Pipeline::create_pipeline_layout( const Context& cntx ) const
	{
		const vk::PipelineLayoutCreateInfo ci( {}, *descriptor_set_layouts );
		return vk::raii::PipelineLayout( cntx.device, ci );
	}

	vk::raii::Pipeline Pipeline::create_pipeline(
		const Context& cntx,
		const std::string init_function_name ) const
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

	vk::raii::DescriptorSets Pipeline::create_descriptor_sets( const Context& cntx ) const
	{
		const vk::DescriptorSetAllocateInfo alloc_info( *pool, *descriptor_set_layouts );
		return vk::raii::DescriptorSets( cntx.device, alloc_info );
	}
}


