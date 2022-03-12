#ifndef FGL_VULKAN_PIPELINEINFO_HPP_INCLUDED
#define FGL_VULKAN_PIPELINEINFO_HPP_INCLUDED

#include <vulkan/vulkan.h>

class RenderPass
{
	VkRenderPass renderPass;

	VkDevice& device;

  public:
	operator VkRenderPass&()
	{
		return renderPass;
	}

	operator VkRenderPass*()
	{
		return &renderPass;
	}

	RenderPass(
		VkDevice& device,
		uint32_t attachementCount,
		VkAttachmentDescription& attach,
		uint32_t subpassCount,
		VkSubpassDescription& subpass )
		: device( device )
	{
		const VkRenderPassCreateInfo info {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = nullptr,
			.attachmentCount = attachementCount,
			.pAttachments = &attach,
			.subpassCount = subpassCount,
			.pSubpasses = &subpass };

		if ( vkCreateRenderPass( device, &info, nullptr, &renderPass ) != VK_SUCCESS )
		{
			throw std::runtime_error( "Failed to create VkRenderPass" );
		}
	}

	~RenderPass()
	{
		vkDestroyRenderPass( device, renderPass, nullptr );
	}
};

#endif /* FGL_VULKAN_PIPELINEINFO_HPP_INCLUDED */
