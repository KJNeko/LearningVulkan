#ifndef FGL_VULKAN_INTERNAL_VERSION_HPP_INCLUDED
#define FGL_VULKAN_INTERNAL_VERSION_HPP_INCLUDED

#include <cstdint>
#include <iostream>

namespace fgl::vulkan::internal
{

/* decomposes a version number into it's components (major, minor, patch)
and declares an ostream operator<<()*/
struct Version
{
	const uint32_t major;
	const uint32_t minor;
	const uint32_t patch;

	[[nodiscard]] explicit Version(const uint32_t enumerated_version);

	friend std::ostream& operator<<(std::ostream&, const Version&);
};

// performs a version check (throws if unsupported)
struct VersionInfo
{
	const Version loaded;
	const Version target;

	[[nodiscard]] explicit VersionInfo(
		const uint32_t enumerated_loaded,
		const uint32_t enumerated_target);
};

}

#endif /* FGL_VULKAN_INTERNAL_VERSION_HPP_INCLUDED */
