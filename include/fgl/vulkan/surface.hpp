#ifndef FGL_VULKAN_SURFACE_HPP_INCLUDED
#define FGL_VULKAN_SURFACE_HPP_INCLUDED

#include <vulkan/vulkan.h>

#include "fgl/vulkan/instance.hpp"

namespace fgl::vulkan
{

class Surface
{
	VkSurfaceKHR surface;

	fgl::vulkan::Instance& instance;

	Surface( Surface const& rhs ) = delete;
	Surface& operator=( Surface const& rhs ) = delete;


  public:
	operator VkSurfaceKHR&()
	{
		return surface;
	}

	operator VkSurfaceKHR*()
	{
		return &surface;
	}

	Surface( fgl::vulkan::Instance& instanceref, GLFWwindow* window )
		: instance( instanceref )
	{
#ifdef __linux__
		auto result =
			glfwCreateWindowSurface( instanceref, window, nullptr, &surface );
		if ( result != VK_SUCCESS )
		{
			throw std::runtime_error(
				"Failed to create window surface in fgl::vulkan::Surface.\n\tError code:" +
				std::to_string( static_cast<int>( result ) ) );
		}

#elif _WIN32
		// TODO ensure this works
		VkWin32SurfaceCreateInfoKHR createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = glfwGetWin32Window( window );
		createInfo.hinstance = GetModuleHandle( nullptr );

		if ( vkCreateWin32SurfaceKHR(
				 instance, &createInfo, nullptr, &surface ) != VK_SUCCESS )
		{
			throw std::runtime_error( "failed to create window surface!" );
		}
#else
		throw std::runtime_error(
			"Compiling in wrong environment. Only supports UNIX/LINUX and Windows" );
#endif
	}

	~Surface()
	{
		vkDestroySurfaceKHR( instance, surface, nullptr );
	}
};

} // namespace fgl::vulkan


#endif /* FGL_VULKAN_SURFACE_HPP_INCLUDED */
