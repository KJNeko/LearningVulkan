#ifndef FGL_VULKAN_PIPELINEINFO_HPP_INCLUDED
#define FGL_VULKAN_PIPELINEINFO_HPP_INCLUDED

#include <vulkan/vulkan.h>

class RenderPass
{
	VkRenderPass renderPass { nullptr };

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

	RenderPass( const RenderPass& ) = delete;
	RenderPass operator=( const RenderPass& ) = delete;
	RenderPass() = delete;

	RenderPass(
		VkDevice& indevice,
		uint32_t attachementCount,
		VkAttachmentDescription& attach,
		uint32_t subpassCount,
		VkSubpassDescription& subpass,
		uint32_t dependencyCount = 0,
		VkSubpassDependency* dependencies = nullptr,
		VkRenderPassCreateFlags flags = VkRenderPassCreateFlags() )
		: device( indevice )
	{
		const VkRenderPassCreateInfo info {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = nullptr,
			.flags = flags,
			.attachmentCount = attachementCount,
			.pAttachments = &attach,
			.subpassCount = subpassCount,
			.pSubpasses = &subpass,
			.dependencyCount = dependencyCount,
			.pDependencies = dependencies };

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

class Pipeline
{
	VkPipeline graphicsPipeline { nullptr };

	VkDevice& device;

	Pipeline( const Pipeline& ) = delete;
	Pipeline operator=( const Pipeline& ) = delete;
	Pipeline() = delete;

  public:
	operator VkPipeline&()
	{
		return graphicsPipeline;
	}

	operator VkPipeline*()
	{
		return &graphicsPipeline;
	}

	Pipeline(
		VkDevice& indevice,
		VkPipelineCache pipelineCache,
		uint32_t createInfoCount,
		const VkGraphicsPipelineCreateInfo* createInfo,
		const VkAllocationCallbacks* pAllocator )
		: device( indevice )
	{
		if ( vkCreateGraphicsPipelines(
				 device,
				 pipelineCache,
				 createInfoCount,
				 createInfo,
				 pAllocator,
				 &graphicsPipeline ) != VK_SUCCESS )
		{
			throw std::runtime_error( "Failed to create VkPipeline" );
		}
	}

	~Pipeline()
	{
		vkDestroyPipeline( device, graphicsPipeline, nullptr );
	}
};

#endif /* FGL_VULKAN_PIPELINEINFO_HPP_INCLUDED */
