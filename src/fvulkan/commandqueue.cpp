#include <fgl/vulkan/commandqueue.hpp>

namespace fgl::vulkan
{

	namespace internal
	{
		auto create_command_buffer(
			const vk::raii::Device& device,
			const vk::raii::CommandPool& command_pool,
			const uint32_t buffer_count = 1 )
		{
			const vk::CommandBufferAllocateInfo alloc_info(
				*command_pool, vk::CommandBufferLevel::ePrimary, buffer_count
			);
			return std::move( vk::raii::CommandBuffers( device, alloc_info ).front() );
		}
	} // namespace internal

	CommandQueue::CommandQueue(
		const fgl::vulkan::Context& context,
		const fgl::vulkan::Pipeline& pipeline,
		const vk::CommandBufferUsageFlagBits flags,
		const uint32_t groupCountX,
		const uint32_t groupCountY,
		const uint32_t groupCountZ )
		:
		pool(
			context.device,
			vk::CommandPoolCreateInfo( {}, context.queue_family_index )
		),
		buffer( internal::create_command_buffer( context.device, pool ) )
	{
		buffer.begin( { flags } );
		buffer.bindPipeline( vk::PipelineBindPoint::eCompute, *pipeline.pipeline );

		std::vector<vk::DescriptorSet> vec;
		vec.reserve( std::ranges::size( pipeline.sets ) );
		for( const auto& layout : pipeline.sets )
			vec.emplace_back( *layout );

		constexpr uint32_t first_set { 0 };
		buffer.bindDescriptorSets(
			vk::PipelineBindPoint::eCompute,
			*pipeline.layout,
			first_set,
			vec,
			nullptr // dynamic offsets
		);

		buffer.dispatch( groupCountX, groupCountY, groupCountZ );
		buffer.end();
	}

} // namespace fgl::vulkan
