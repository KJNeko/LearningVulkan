#ifndef ED4917DF_0E3F_4B8F_BF5A_512369712524
#define ED4917DF_0E3F_4B8F_BF5A_512369712524

#include <filesystem>
#include <fstream>

namespace fgl::vulkan
{
namespace internal
{
	std::vector<char> readFile( std::filesystem::path path )
	{
		if ( !std::filesystem::exists( path ) )
		{
			throw std::runtime_error(
				"fgl::vulkan::internal::readFile() ERROR: Failed to find the file. \n\t\tFilepath:" +
				path.string() );
		}
		if ( std::ifstream ifs( path, std::ios::ate | std::ios::binary ); ifs )
		{
			size_t fileSize = static_cast<size_t>( ifs.tellg() );
			std::vector<char> buffer( fileSize );

			ifs.seekg( 0 );
			ifs.read( buffer.data(), static_cast<long int>( fileSize ) );

			return buffer;
		}
		throw std::runtime_error( "Failed to read the file" );
	}


} // namespace internal

class ShaderModule
{
	VkShaderModule shaderModule {};

	fgl::vulkan::Device& parentDevice;

	ShaderModule( const ShaderModule& ) = delete;
	ShaderModule operator=( const ShaderModule& ) = delete;
	ShaderModule() = delete;

  public:
	operator VkShaderModule&()
	{
		return shaderModule;
	}

	ShaderModule( fgl::vulkan::Device& device, std::filesystem::path shaderPath )
		: parentDevice( device )
	{
		createShader( device, internal::readFile( shaderPath ) );
	}

	void createShader( fgl::vulkan::Device& device, std::vector<char> code )
	{
		VkShaderModuleCreateInfo createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(
			reinterpret_cast<void*>( code.data() ) );

		if ( vkCreateShaderModule(
				 device, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS )
		{
			throw std::runtime_error( "Failed to create the shader module" );
		}
	}

	~ShaderModule()
	{
		vkDestroyShaderModule( parentDevice, shaderModule, nullptr );
	}
};

} // namespace fgl::vulkan


#endif /* ED4917DF_0E3F_4B8F_BF5A_512369712524 */
