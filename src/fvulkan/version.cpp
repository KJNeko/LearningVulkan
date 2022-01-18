
#include <vulkan/vulkan_raii.hpp> // VK_VERSION_MAJOR etc

#include "../../include/fgl/vulkan/internal/version.hpp"

namespace fgl::vulkan::internal
{

	Version::Version( const uint32_t enumerated_version )
		:
		major( VK_VERSION_MAJOR( enumerated_version ) ),
		minor( VK_VERSION_MINOR( enumerated_version ) ),
		patch( VK_VERSION_PATCH( enumerated_version ) )
	{}

	std::ostream& operator<<( std::ostream& os, const Version& v )
	{
		return ( os << v.major << '.' << v.minor << '.' << v.patch );
	}


	VersionInfo::VersionInfo(
		const uint32_t enumerated_loaded,
		const uint32_t enumerated_target )
		:
		loaded( enumerated_loaded ),
		target( enumerated_target )
	{
		if(
			loaded.major < target.major
			|| ( loaded.major == target.major
			&& loaded.minor < target.minor ) )
		{
			std::stringstream ss;
			ss
				<< "Vulkan API " << target << " is not supported"
				<< " (system has version: " << loaded << ')';
			throw std::runtime_error( ss.str() );
		}
	}

} // namespace fgl::vulkan::internal {
