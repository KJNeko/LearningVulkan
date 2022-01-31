#include <concepts>
#include <cstring>
#include <iostream> // cout, cerr, endl
#include <iostream>
#include <ranges>
#include <string_view>
#include <thread>
#include <vector>

#include <vulkan/vulkan.h>

#include "stopwatch.hpp"

#include <fgl/vulkan.hpp>


// GLFW
#include <GLFW/glfw3.h>

int main()
try
{
	stopwatch::Stopwatch mainwatch( "Main" );
	mainwatch.start();

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

	mainwindow.mainLoop();

	mainwatch.stop();
	std::cout << '\n' << mainwatch << std::endl;
}
catch ( const std::exception& e )
{
	std::cerr << "\n\n Exception caught:\n\t" << e.what()
			  << std::endl;
	std::abort();
}
catch ( ... )
{
	std::cerr << "\n\n An unknown error has occured.\n";
	std::abort();
}
