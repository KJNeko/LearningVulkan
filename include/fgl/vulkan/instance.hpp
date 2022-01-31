#ifndef FGL_VULKAN_INSTANCE_HPP_INCLUDED
#define FGL_VULKAN_INSTANCE_HPP_INCLUDED

#include <vector>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>


namespace fgl::vulkan
{
// Wrapper for the vkInstance object
class Instance
{
	VkInstance instance { nullptr };

	Instance()						= delete; // Default
	Instance( Instance const& rhs ) = delete;
	Instance& operator=( Instance const& rhs ) = delete;

  public:
	operator VkInstance&()
	{
		return instance;
	}

	Instance(
		std::string applicationName,
		uint32_t applicationVersion,
		std::string engineName,
		uint32_t engineVersion,
		uint32_t apiVersion,
		std::vector<const char*> extensions,
		std::vector<const char*> layers,
		fgl::vulkan::Debug debugInfo = {} )
	{
		// Initalize glfw
		glfwInit();

		// Check if vulkan is supported
		if ( !glfwVulkanSupported() )
		{
			throw std::runtime_error( "Vulkan is not supported by glfw" );
		}

		// Check for layer support
		auto validLayers = [ & ]() {
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

			std::vector<VkLayerProperties> availableLayers( layerCount );
			vkEnumerateInstanceLayerProperties(
				&layerCount, availableLayers.data() );

			for ( const char* layerName : layers )
			{
				bool layerFound = false;
				for ( const auto& layerProperties : availableLayers )
				{
					if ( std::strcmp( layerName, layerProperties.layerName ) == 0 )
					{
						layerFound = true;
						break;
					}
				}
				if ( !layerFound )
				{
					return false;
				}
			}
			return true;
		}();

		if ( !validLayers )
		{
			throw std::runtime_error(
				"Validation layers requested are not available" );
		}

		// Create the appinfo and instance info
		VkApplicationInfo appInfo {};
		appInfo.sType			   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName   = applicationName.c_str();
		appInfo.applicationVersion = applicationVersion;
		appInfo.pEngineName		   = engineName.c_str();
		appInfo.engineVersion	   = engineVersion;
		appInfo.apiVersion		   = apiVersion;

		VkInstanceCreateInfo createInfo {};
		createInfo.sType			= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Add the required extensions and extenions we added.
		auto glfwextensions = [ & ]() {
			uint32_t glfwExtensionCount { 0 };
			const char** glfwExtensions;
			glfwExtensions =
				glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

			if ( glfwExtensions == NULL )
			{
				throw std::runtime_error(
					"Failed to get required glfw extensions, Returned NULL" );
			}

			if ( glfwExtensionCount == 0 )
			{
				throw std::runtime_error( "Failed to get required glfw extensions" );
			}

			std::vector<const char*> tempextensions(
				glfwExtensions, glfwExtensions + glfwExtensionCount );

			for ( const auto& extention : extensions )
			{
				tempextensions.push_back( extention );
			}

			return tempextensions;
		}();

		createInfo.enabledExtensionCount =
			static_cast<uint32_t>( glfwextensions.size() );
		createInfo.ppEnabledExtensionNames = glfwextensions.data();


		// TODO Debug shit here

		if ( vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS )
		{
			throw std::runtime_error(
				"Failed to create instance in fgl::vulkan::Instance" );
		}
	}

	~Instance()
	{
		// Cleanup VkInstance
		vkDestroyInstance( instance, nullptr );
	}
};
} // namespace fgl::vulkan

#endif /* FGL_VULKAN_INSTANCE_HPP_INCLUDED */
