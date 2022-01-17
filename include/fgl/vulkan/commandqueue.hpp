#ifndef FGL_VULKAN_COMMANDQUEUE_HPP_INCLUDED
#define FGL_VULKAN_COMMANDQUEUE_HPP_INCLUDED

#include <vector>
#include <ranges>

#include "./context.hpp"
#include "./pipeline.hpp"

namespace fgl::vulkan {

struct CommandQueue
{
	const vk::raii::CommandPool pool;
	const vk::raii::CommandBuffer buffer;

	[[nodiscard]] explicit CommandQueue(
		const fgl::vulkan::Context& context,
		const fgl::vulkan::Pipeline& pipeline,
		const vk::CommandBufferUsageFlagBits flags,
		const uint32_t groupCountX,
		const uint32_t groupCountY = 1,
		const uint32_t groupCountZ = 1);
};

} // namespace fgl::vulkan

#endif /* FGL_VULKAN_COMMANDQUEUE_HPP_INCLUDED */
