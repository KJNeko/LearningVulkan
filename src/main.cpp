#include <concepts>
#include <cstring>
#include <iostream> // cout, cerr, endl
#include <iostream>
#include <ranges>
#include <string_view>
#include <thread>
#include <vector>

#include <vulkan/vulkan.h>

#include <fgl/vulkan.hpp>


// GLFW
#include <GLFW/glfw3.h>

int main()
try
{

	std::string appName { "Vulkan Testing" };
	uint32_t version { VK_MAKE_VERSION( 1, 0, 0 ) };
	std::string engineName { "FGL Engine" };
	uint32_t APIVersion { VK_API_VERSION_1_0 };

	std::vector<const char*> extensions {
		"VK_EXT_debug_utils", "VK_KHR_xcb_surface" };
	std::vector<const char*> layers { "VK_LAYER_KHRONOS_validation" };


	// Setup the instance
	fgl::vulkan::Instance maininst(
		appName, version, engineName, version, APIVersion, extensions, layers );

	// Window hints for GLFW
	std::vector<std::pair<int, int>> windowHints {
		{ GLFW_CLIENT_API, GLFW_NO_API }, { GLFW_RESIZABLE, GLFW_FALSE } };

	// Get the physical device
	fgl::vulkan::PhysicalDevice physicalDevice(
		maininst, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT );

	// Setup the rendering window
	fgl::vulkan::Window mainwindow(
		maininst, physicalDevice, windowHints, { 800, 600 }, "Vulkan Testing" );

	std::vector<const char*> deviceExtensions { "VK_KHR_swapchain" };

	// Setup the logicalDevice
	fgl::vulkan::Device device(
		physicalDevice,
		VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT,
		deviceExtensions,
		mainwindow,
		0.0f );

	// Get the shaders

	fgl::vulkan::ShaderModule frag(
		device,
		std::filesystem::path(
			"/home/kj16609/Desktop/Projects/vulkan/src/shaders/shader.frag.spv" ) );
	fgl::vulkan::ShaderModule vert(
		device,
		std::filesystem::path(
			"/home/kj16609/Desktop/Projects/vulkan/src/shaders/shader.vert.spv" ) );

	// graphics pipeline

	VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vert;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = frag;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>( device.swapChain.swapChainExtent.width );
	viewport.height =
		static_cast<float>( device.swapChain.swapChainExtent.height );
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor {};
	scissor.offset = { 0, 0 };
	scissor.extent = device.swapChain.swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f;		   // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f;	   // Optional

	VkPipelineMultisampleStateCreateInfo multisampling {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;			// Optional
	multisampling.pSampleMask = nullptr;			// Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE;		// Optional


	VkPipelineColorBlendAttachmentState colorBlendAttachment {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;	 // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;			 // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;	 // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;			 // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[ 0 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 1 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 2 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 3 ] = 0.0f; // Optional


	VkPipelineLayout pipelineLayout;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;			  // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr;		  // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;	  // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if ( vkCreatePipelineLayout(
			 device, &pipelineLayoutInfo, nullptr, &pipelineLayout ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to create pipeline layout!" );
	}

	// END PIPELINE

	// Render pass
	VkAttachmentDescription colorAttachment {};
	colorAttachment.format = device.swapChain.swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;


	RenderPass renderPass( device, 1, colorAttachment, 1, subpass );

	// Graphics Pipeline object
	VkGraphicsPipelineCreateInfo pipelineInfo {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1;			  // Optional

	Pipeline graphicsPipeline( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr );

	std::vector<VkFramebuffer> swapChainFramebuffers;
	swapChainFramebuffers.resize( device.swapChain.swapChainImages.size() );

	// TODO I guess I forgot to put this somewhere
	fgl::vulkan::ImageViews imageViews( device.swapChain, device );


	for ( size_t i = 0; i < device.swapChain.swapChainImages.size(); i++ )
	{
		VkImageView attachments[] = { imageViews[ i ] };

		VkFramebufferCreateInfo framebufferInfo {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = device.swapChain.swapChainExtent.width;
		framebufferInfo.height = device.swapChain.swapChainExtent.height;
		framebufferInfo.layers = 1;

		if ( vkCreateFramebuffer(
				 device, &framebufferInfo, nullptr, &swapChainFramebuffers[ i ] ) !=
			 VK_SUCCESS )
		{
			throw std::runtime_error( "failed to create framebuffer!" );
		}
	}

	VkCommandPool commandPool;

	// QueueFamilyIndices queueFamilyIndices = findQueueFamilies( physicalDevice
	// );

	VkCommandPoolCreateInfo poolInfo;
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = fgl::vulkan::internal::findIndexForFlags(
		physicalDevice, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT );
	poolInfo.flags = 0; // Optional

	if ( vkCreateCommandPool( device, &poolInfo, nullptr, &commandPool ) !=
		 VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to create command pool" );
	}

	std::vector<VkCommandBuffer> commandBuffers;

	const int MAX_FRAMES_IN_FLIGHT = 2;
	commandBuffers.resize( MAX_FRAMES_IN_FLIGHT );


	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	allocInfo.commandBufferCount = static_cast<uint32_t>( commandBuffers.size() );

	if ( vkAllocateCommandBuffers( device, &allocInfo, commandBuffers.data() ) !=
		 VK_SUCCESS )
	{
		throw std::runtime_error( "failed to allocate command buffers!" );
	}

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	imageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
	renderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
	inFlightFences.resize( MAX_FRAMES_IN_FLIGHT );

	VkSemaphoreCreateInfo semaphoreInfo {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		if ( vkCreateSemaphore(
				 device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[ i ] ) !=
				 VK_SUCCESS ||
			 vkCreateSemaphore(
				 device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[ i ] ) !=
				 VK_SUCCESS ||
			 vkCreateFence( device, &fenceInfo, nullptr, &inFlightFences[ i ] ) !=
				 VK_SUCCESS )
		{

			throw std::runtime_error(
				"failed to create synchronization objects for a frame!" );
		}
	}

	uint32_t currentFrame = 0;

	auto recordCommandBuffer = [ & ](
								   VkCommandBuffer commandBuffer,
								   uint32_t imageIndex ) {
		VkCommandBufferBeginInfo beginInfo {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if ( vkBeginCommandBuffer( commandBuffer, &beginInfo ) != VK_SUCCESS )
		{
			throw std::runtime_error( "failed to begin recording command buffer!" );
		}

		VkRenderPassBeginInfo renderPassInfo {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[ imageIndex ];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = device.swapChain.swapChainExtent;

		VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(
			commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

		vkCmdBindPipeline(
			commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );

		vkCmdDraw( commandBuffer, 3, 1, 0, 0 );

		vkCmdEndRenderPass( commandBuffer );

		if ( vkEndCommandBuffer( commandBuffer ) != VK_SUCCESS )
		{
			throw std::runtime_error( "failed to record command buffer!" );
		}
	};

	auto drawFrame = [ & ]() {
		vkWaitForFences(
			device, 1, &inFlightFences[ currentFrame ], VK_TRUE, UINT64_MAX );
		vkResetFences( device, 1, &inFlightFences[ currentFrame ] );

		uint32_t imageIndex;
		vkAcquireNextImageKHR(
			device,
			device.swapChain,
			UINT64_MAX,
			imageAvailableSemaphores[ currentFrame ],
			VK_NULL_HANDLE,
			&imageIndex );

		vkResetCommandBuffer(
			commandBuffers[ currentFrame ], /*VkCommandBufferResetFlagBits*/ 0 );
		recordCommandBuffer( commandBuffers[ currentFrame ], imageIndex );

		VkSubmitInfo submitInfo {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[ currentFrame ] };
		VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[ currentFrame ];

		VkSemaphore signalSemaphores[] = {
			renderFinishedSemaphores[ currentFrame ] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if ( vkQueueSubmit(
				 device.getQueue(), 1, &submitInfo, inFlightFences[ currentFrame ] ) !=
			 VK_SUCCESS )
		{
			throw std::runtime_error( "failed to submit draw command buffer!" );
		}

		VkPresentInfoKHR presentInfo {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { device.swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR( device.getPresentQueue(), &presentInfo );

		currentFrame = ( currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
	};


	// Should always be able to be threaded.
	while ( !glfwWindowShouldClose( mainwindow.windowHandle ) )
	{
		glfwPollEvents();
		drawFrame();
	}

	for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
	{
		vkDestroySemaphore( device, renderFinishedSemaphores[ i ], nullptr );
		vkDestroySemaphore( device, imageAvailableSemaphores[ i ], nullptr );
		vkDestroyFence( device, inFlightFences[ i ], nullptr );
	}

	for ( auto framebuffer : swapChainFramebuffers )
	{
		vkDestroyFramebuffer( device, framebuffer, nullptr );
	}
	vkDestroyCommandPool( device, commandPool, nullptr );
	vkDestroyPipelineLayout( device, pipelineLayout, nullptr );
}
catch ( const std::exception& e )
{
	std::cerr << "\n\n Exception caught:\n\t" << e.what() << std::endl;
	std::abort();
}
catch ( ... )
{
	std::cerr << "\n\n An unknown error has occured.\n";
	std::abort();
}
