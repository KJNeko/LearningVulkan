

#include "fglVulkan.hpp"

using namespace fgl::vulkan;

int main()
{
    AppInfo info
    (
        "Fuk",
        0,
        "NO",
        0,
        VK_API_VERSION_1_2,
        { "VK_LAYER_KHRONOS_validation" },
        {}
    );

    Vulkan inst( info );




}