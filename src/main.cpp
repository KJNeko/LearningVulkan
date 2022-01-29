#include <iostream> // cout, cerr, endl
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "stopwatch.hpp"

#include <fgl/vulkan.hpp>


int main()
try
{
	stopwatch::Stopwatch mainwatch( "Main" );
	mainwatch.start();


	mainwatch.stop();
	std::cout << '\n' << mainwatch << std::endl;
}
catch ( const vk::SystemError& e )
{
	std::cerr << "\n\n Vulkan system error code:\t" << e.code()
			  << "\n\t error:" << e.what() << std::endl;
	std::abort();
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
