#ifndef A7DDB718_E48C_45D0_8866_F5725D7859FD
#define A7DDB718_E48C_45D0_8866_F5725D7859FD

#include "./fgl/vulkan/device.hpp"
#include "./fgl/vulkan/swapchain.hpp"
#include <fgl/vulkan.hpp>
#include <vector>

namespace fgl::vulkan
{

class ImageViews
{
	std::vector<VkImageView> swapChainImageViews;

	VkDevice& parentDevice;

	ImageViews() = delete;

	ImageViews( fgl::vulkan::Swapchain& swapchain, fgl::vulkan::Device& device )
		: parentDevice( device )
	{
		swapChainImageViews.resize( swapchain.swapChainImages.size() );

		for ( size_t i = 0; i < swapchain.swapChainImages.size(); ++i )
		{
			VkImageViewCreateInfo createInfo {};

			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapchain.swapChainImages[ i ];

			// This should all be accessable later on with a seperate structure
			// to feed into it. Or mode selection. But for basic graphics
			// pipelines this is fine.
			createInfo.viewType		= VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format		= swapchain.swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel   = 0;
			createInfo.subresourceRange.levelCount	   = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount	   = 1;
			if ( vkCreateImageView(
					 device, &createInfo, nullptr, &swapChainImageViews[ i ] ) !=
				 VK_SUCCESS )
			{
				throw std::runtime_error( "Failed to create an image view" );
			}
		}
	}

	~ImageViews()
	{
		for ( auto& imageView : swapChainImageViews )
		{
			vkDestroyImageView( parentDevice, imageView, nullptr );
		}
	}
};

} // namespace fgl::vulkan

#endif /* A7DDB718_E48C_45D0_8866_F5725D7859FD */
