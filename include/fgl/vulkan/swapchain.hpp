#ifndef F66B9F5B_419D_4A4D_802A_1391E7DC9B71
#define F66B9F5B_419D_4A4D_802A_1391E7DC9B71

#include "./fgl/vulkan/window.hpp"
#include <fgl/vulkan.hpp>

namespace fgl::vulkan
{

class Swapchain
{
	VkSwapchainKHR swapChain;

	VkDevice& parentDevice;

  public:
	operator VkSwapchainKHR()
	{
		return swapChain;
	}

	operator VkSwapchainKHR&()
	{
		return swapChain;
	}

	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;


	Swapchain(
		VkDevice& device,
		fgl::vulkan::PhysicalDevice& physicalDevice,
		fgl::vulkan::Window& window )
		: parentDevice( device )
	{
		// Swapchain
		VkSurfaceCapabilitiesKHR capabilities;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			physicalDevice, window.surface, &capabilities );

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			physicalDevice, window.surface, &formatCount, nullptr );

		std::vector<VkSurfaceFormatKHR> formats( formatCount );

		if ( formatCount != 0 )
		{
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				physicalDevice, window.surface, &formatCount, formats.data() );
		}
		else
		{
			throw std::runtime_error( "Formats is empty" );
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physicalDevice, window.surface, &presentModeCount, nullptr );

		std::vector<VkPresentModeKHR> presentModes( presentModeCount );

		if ( presentModeCount != 0 )
		{
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				physicalDevice,
				window.surface,
				&presentModeCount,
				presentModes.data() );
		}
		else
		{
			throw std::runtime_error( "Surface Present Modes is empty" );
		}

		// Choose swap surface Format
		auto selectedFormat = [ & ]() {
			for ( const auto& format : formats )
			{
				if ( format.format == VK_FORMAT_B8G8R8A8_SRGB &&
					 format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
				{
					return format;
				}
			}

			throw std::runtime_error( "Failed to find a valid format" );
		}();

		// Switch presentation mode
		/*
		VK_PRESENT_MODE_IMMEDIATE_KHR : Images are submitted as soon as
		possible. Tearing sometimes.

		VK_PRESENT_MODE_FIFO_KHR : swap chain works as a Queue. 'vsync'. Sync
		period is 'vertical blank'

		VK_PRESENT_MODE_RELAXED_KHR : Works like FIFO but transfers instantly if
		the queue is empty

		VK_PRESENT_MODE_MAILBOX_KHR : Images already queued are replaced with
		newer ones. "triple buffering"
		*/

		// Chose best mode

		auto presentationMode = [ & ]() {
			for ( const auto& availableMode : presentModes )
			{
				if ( availableMode == VK_PRESENT_MODE_MAILBOX_KHR )
				{
					return availableMode;
				}
			}
			// Guaranteed to be available but honestly the worst choice
			return VK_PRESENT_MODE_FIFO_KHR;
		}();

		// Figure out swap extent
		auto swapExtent = [ & ]() {
			if ( capabilities.currentExtent.width != UINT32_MAX )
			{
				return capabilities.currentExtent;
			}
			else
			{
				int width { 0 }, height { 0 };
				glfwGetFramebufferSize( window, &width, &height );

				VkExtent2D actualExtent = {
					static_cast<uint32_t>( width ),
					static_cast<uint32_t>( height ) };


				// Clamp down the width and height to the proper sizes.
				actualExtent.width = std::clamp(
					actualExtent.width,
					capabilities.minImageExtent.width,
					capabilities.maxImageExtent.width );
				actualExtent.height = std::clamp(
					actualExtent.height,
					capabilities.minImageExtent.height,
					capabilities.maxImageExtent.height );

				return actualExtent;
			}

			throw std::runtime_error( "Failed to find a swap extent" );
		}();

		uint32_t imageCount = capabilities.minImageCount + 1;

		if ( capabilities.maxImageCount > 0 &&
			 imageCount > capabilities.maxImageCount )
		{
			imageCount = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo {};
		createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = window.surface;

		createInfo.minImageCount	= imageCount;
		createInfo.imageFormat		= selectedFormat.format;
		createInfo.imageColorSpace	= selectedFormat.colorSpace;
		createInfo.imageExtent		= swapExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto graphicsIndex = fgl::vulkan::internal::findIndexForFlags(
			physicalDevice, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT );

		auto presentIndex = fgl::vulkan::internal::findIndexForFlags(
			physicalDevice,
			VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT,
			true,
			window.surface );

		uint32_t queueFamilyIndicies[] = { graphicsIndex, presentIndex };

		if ( graphicsIndex != presentIndex )
		{
			createInfo.imageSharingMode		 = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices	 = queueFamilyIndicies;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform	  = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode	  = presentationMode;
		createInfo.clipped		  = VK_TRUE;
		createInfo.oldSwapchain	  = VK_NULL_HANDLE;

		if ( vkCreateSwapchainKHR( device, &createInfo, nullptr, &swapChain ) !=
			 VK_SUCCESS )
		{
			throw std::runtime_error( "Failed to create the swap chain" );
		}

		vkGetSwapchainImagesKHR( device, swapChain, &imageCount, nullptr );
		swapChainImages.resize( imageCount );
		vkGetSwapchainImagesKHR(
			device, swapChain, &imageCount, swapChainImages.data() );

		swapChainImageFormat = selectedFormat.format;
		swapChainExtent		 = swapExtent;
	}

	~Swapchain()
	{
		vkDestroySwapchainKHR( parentDevice, swapChain, nullptr );
	}
};


} // namespace fgl::vulkan


#endif /* F66B9F5B_419D_4A4D_802A_1391E7DC9B71 */
