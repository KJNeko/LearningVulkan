#ifndef F6EB9FA4_E75A_4869_9036_C6BFAE2F5345
#define F6EB9FA4_E75A_4869_9036_C6BFAE2F5345

#include "./fgl/vulkan/debug.hpp"
#include "./fgl/vulkan/instance.hpp"
#include "./fgl/vulkan/physicaldevice.hpp"
#include "./fgl/vulkan/surface.hpp"
#include <GLFW/glfw3.h>
#include <fgl/vulkan.hpp>
#include <vulkan/vulkan.h>


void error_callback( int code, const char* description )
{
	std::cout << "Code: " << code << "\n\tMessage: " << description << std::endl;
}

namespace fgl::vulkan
{

class Window
{
	// Window has the handle to the window itself, Aswell as a handle to the
	// vulkan instance it is running off of


	fgl::vulkan::Instance& instance;
	fgl::vulkan::PhysicalDevice& device;

  public:
	GLFWwindow* windowHandle { nullptr };

  public:
	fgl::vulkan::Surface surface;

	operator GLFWwindow*()
	{
		return windowHandle;
	}

  private:
	Window( Window const& rhs ) = delete;
	Window& operator=( Window const& rhs ) = delete;


	Window() = delete; // Default


	// fgl::vulkan::Debug debugMessenger; TODO

	template <std::ranges::forward_range T>
	requires std::same_as<std::pair<int, int>, std::ranges::range_value_t<T>>
	auto createWindowHandle(
		T windowHints,
		std::pair<int, int> windowSize,
		std::string windowName,
		GLFWmonitor* monitor,
		GLFWwindow* share )
	{
		for ( const auto& [ hint, value ] : windowHints )
		{
			glfwWindowHint( hint, value );
		}

		glfwSetErrorCallback( error_callback );

		auto result = glfwCreateWindow(
			windowSize.first, windowSize.second, windowName.c_str(), monitor, share );

		if ( result == NULL )
		{
			throw std::runtime_error( "Failed to create window" );
		}
		else
			return result;
	}

  public:
	template <std::ranges::forward_range T>
	requires std::same_as<std::pair<int, int>, std::ranges::range_value_t<T>>
	Window(
		fgl::vulkan::Instance& instanceref,
		fgl::vulkan::PhysicalDevice& deviceref,
		T& windowHints,
		std::pair<int, int> windowSize,
		std::string windowName = "",
		GLFWmonitor* monitor = nullptr,
		GLFWwindow* share = nullptr )
		: instance( instanceref ),
		  device( deviceref ),
		  windowHandle( createWindowHandle(
			  windowHints, windowSize, windowName, monitor, share ) ),
		  surface( instance, windowHandle )
	{}

	void mainLoop()
	{
		// Should always be able to be threaded.
		while ( !glfwWindowShouldClose( windowHandle ) )
		{
			glfwPollEvents();
		}
	}

	~Window()
	{ // Delete all internal objects
		glfwDestroyWindow( windowHandle );

		glfwTerminate();
	}
};

} // namespace fgl::vulkan

#endif /* F6EB9FA4_E75A_4869_9036_C6BFAE2F5345 */
