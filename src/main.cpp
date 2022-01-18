#include <cstdlib> // abort, EXIT_SUCCESS
#include <cstdint>
#include <array>
#include <utility> // move, pair
#include <algorithm> // find_if
#include <sstream> // used for API version exception
#include <filesystem>
#include <fstream>
#include <bitset>

#include <string_view>
#include <iostream> // cout, cerr, endl

#include <vulkan/vulkan_raii.hpp>

#include "stopwatch.hpp"

#include <fgl/vulkan.hpp>

#include <fgl/types/range_wrapper.hpp>

// could use StructureChain, but it would be more verbose?
// https://github.com/KhronosGroup/Vulkan-Hpp/search?q=StructureChain

auto create_waitable_fence(
	const vk::raii::Device& device,
	const vk::raii::CommandBuffer& command_buffer,
	const uint32_t queue_family_index,
	const uint32_t queue_index = 0 )
{
	vk::raii::Fence fence { device.createFence( {} ) };
	const vk::raii::Queue queue { device.getQueue( queue_family_index, queue_index ) };
	const vk::SubmitInfo submit_info( nullptr, nullptr, *command_buffer, nullptr );
	queue.submit( submit_info, *fence );
	return fence;
}

int main() try
{
	stopwatch::Stopwatch mainwatch( "Main" );
	mainwatch.start();

	fgl::vulkan::AppInfo info(
		VK_API_VERSION_1_1,
		{ "VK_LAYER_KHRONOS_validation" },
		{},
		1,
		0.0
	);

	fgl::vulkan::Context inst( info );
	inst.print_debug_info();

	constexpr size_t elements = 512;
	constexpr vk::DeviceSize insize = elements * sizeof( uint32_t ) + sizeof( uint32_t );
	constexpr vk::DeviceSize outsize = ( elements * elements ) * sizeof( uint32_t );
	constexpr size_t invocationsPerDispatch = 2;
	constexpr size_t dispatchNum = elements / invocationsPerDispatch;

	//TODO: Add some security checks to ensure we are not dispatching too many calls and allocating too much memory

	//assert( invocationsPerDispatch * invocationsPerDispatch < inst.properties.limits.maxComputeWorkGroupInvocations ); //Too many invocationsPerDispatch

	if( invocationsPerDispatch * invocationsPerDispatch >= inst.properties.limits.maxComputeWorkGroupInvocations )
	{
		throw std::runtime_error( "Too many invocations per dispatch. Lower the elements count" );
	}

	//assert( totalsize < inst.properties.limits.maxMemoryAllocationCount ); //Too many elements allocated

	if( dispatchNum >= inst.properties.limits.maxComputeWorkGroupCount.front() )
	{
		throw std::runtime_error( "dispatch number is too high for supported GPU. Lower the elements count" );
	}

	//assert( dispatchNum < inst.properties.limits.maxComputeWorkGroupCount.front() );

	//Allocate a single memory segment for the buffers being passed in
	/*

	AMD and Nvidia have seperate heap types however...
	AMD DOES have the 256MB cluster of memory that is extremely fast for both CPU and GPU
	(TODO:Research resizable bar for AMD gpus.)
	Perhaps ditching the idea of allocating buffers into a single heap would be a better idea.
	*/


	//eHostVisible is 'slower' then non eHostVisible memory on certian GPUs. If the memory is not needed to be access on the cpu
	//IE: inter-communication/transfer between two shaders then it doesn't need to be visible


	/*
	Flag meanings for vk::MemoryPropertyFlags

	eDeviceLocal : Memory that is optimal for the GPU only, Not mappable.
	eHostVisible : Memory that can be mapped to the host via vkMapMemory
	eHostCoherent : Memory that specieis host cache management (If enabled vkFlush and vkInvalidate at nore required)
	eHostCached : Memory that is cached on the host.

	There is a bunch more but so far they are not important for what I do.
	*/


	constexpr vk::MemoryPropertyFlags flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

	std::vector<fgl::vulkan::Buffer> buffers;
	buffers.reserve( 2 );

	//binding : set

	buffers.emplace_back( inst, insize, vk::BufferUsageFlagBits::eStorageBuffer, vk::SharingMode::eExclusive, 0, flags, vk::DescriptorType::eStorageBuffer );
	buffers.emplace_back( inst, outsize, vk::BufferUsageFlagBits::eStorageBuffer, vk::SharingMode::eExclusive, 1, flags, vk::DescriptorType::eStorageBuffer );


	fgl::vulkan::Pipeline vpipeline( inst, std::filesystem::path( "Square.spv" ), std::string( "main" ), buffers );

	{
		auto& fglbuffer { buffers.at( 0 ) };

		uint32_t* const in_buffer_data {
				reinterpret_cast< uint32_t* > ( fglbuffer.get_memory() )
		};

		in_buffer_data[0] = elements; // matrix size

		auto matrix { fgl::make_range( &in_buffer_data[1], elements ) };

		for( uint32_t i { 0 }; auto & element : matrix )
		{
			element = i++;
		}

		/*
		std::cout << "Input Buffer:" << std::endl;
		for(auto element : matrix)
		{
			std::cout << std::setw( 5 ) << element << " ";
		}
		std::cout << std::endl;
		//*/

		fglbuffer.memory.unmapMemory();
	}

	const fgl::vulkan::CommandQueue command(
		inst,
		vpipeline,
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
		dispatchNum,
		dispatchNum
	);

	const auto fence { create_waitable_fence( inst.device, command.buffer, inst.queue_family_index ) };
	constexpr uint64_t timeout { 5 };
	while( vk::Result::eTimeout == inst.device.waitForFences( { *fence }, VK_TRUE, timeout ) );

	/// PRINT
	/*
	auto out_buffer_ptr = reinterpret_cast< uint32_t* >( buffers.at( 1 ).get_memory() );
	std::cout << "Output Buffer:" << std::endl;
	for( size_t y = 0; y < elements; ++y )// spammy...
	{
		for( size_t x = 0; x < elements; ++x )
		{
			auto index = y * elements + x;
			std::cout << std::setw( 5 ) << out_buffer_ptr[index];
		}
		std::cout << "\n\n" << std::endl;
	}
	buffers.at( 1 ).memory.unmapMemory();
	//*/

	mainwatch.stop();
	std::cout << '\n' << mainwatch << std::endl;
}
catch( const vk::SystemError& e )
{
	std::cerr << "\n\n Vulkan system error code:\t" << e.code() << "\n\t error:" << e.what() << std::endl;
	std::abort();
}
catch( const std::exception& e )
{
	std::cerr << "\n\n Exception caught:\n\t" << e.what() << std::endl;
	std::abort();
}
catch( ... )
{
	std::cerr << "\n\n An unknown error has occured.\n";
	std::abort();
}
