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

#include "fvulkan/context.hpp"
#include "fvulkan/memory.hpp"
#include "fvulkan/pipeline.hpp"

constexpr bool debug_printing {
#ifndef NDEBUG
	true
#else
	false
#endif// NDEBUG
};

using cstring = const char*;



// globals for configuration
constexpr cstring appName { "VulkanCompute" };
constexpr uint32_t appVersion {};
constexpr cstring engineName {};
constexpr uint32_t engineVersion {};
constexpr auto apiVersion { VK_API_VERSION_1_2 };
constexpr std::array layers { "VK_LAYER_KHRONOS_validation" };
constexpr std::array<cstring, 0> extensions {};

// could use StructureChain, but it would be more verbose?
// https://github.com/KhronosGroup/Vulkan-Hpp/search?q=StructureChain

void verify_version_requirements( const vk::raii::Context& context )
{
	const uint32_t loaded_version { context.enumerateInstanceVersion() };
	const uint32_t loader_major { VK_VERSION_MAJOR( loaded_version ) };
	const uint32_t loader_minor { VK_VERSION_MINOR( loaded_version ) };
	const uint32_t target_major { VK_VERSION_MAJOR( apiVersion ) };
	const uint32_t target_minor { VK_VERSION_MINOR( apiVersion ) };

	if( loader_major < target_major
		|| ( loader_major == target_major && loader_minor < target_minor ) )
	{
		std::stringstream ss;
		ss
			<< "Vulkan API " << target_major << '.' << target_minor
			<< " is not supported (system has version: "
			<< loader_major << '.' << loader_minor << ')';
		throw std::runtime_error( ss.str() );
	}
}

auto create_command_pool(
	const vk::raii::Device& device,
	const uint32_t queue_family_index )
{
	const vk::CommandPoolCreateInfo ci( {}, queue_family_index );
	return vk::raii::CommandPool( device, ci );
}

auto createCommandBuffers(
	const vk::raii::Device& device,
	const vk::raii::CommandPool& command_pool,
	const uint32_t buffer_count = 1 )
{
	const vk::CommandBufferAllocateInfo alloc_info(
		*command_pool, vk::CommandBufferLevel::ePrimary, buffer_count
	);
	return vk::raii::CommandBuffers( device, alloc_info );
}

auto create_command_buffer(
	const vk::raii::Device& device,
	const vk::raii::CommandPool& command_pool )
{
	return std::move( createCommandBuffers( device, command_pool ).front() );
}

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

/// <DEBUG PRINTING>
void debug_print( const std::string_view message )
{
	if constexpr( debug_printing )
		std::cout << message << std::endl;
}

void debug_print(
	const vk::raii::Context& context,
	const vk::raii::PhysicalDevice& physical_device,
	const uint32_t queue_family_index )
{
	if constexpr( debug_printing )
	{
		const uint32_t loaded_version { context.enumerateInstanceVersion() };
		const uint32_t loader_major { VK_VERSION_MAJOR( loaded_version ) };
		const uint32_t loader_minor { VK_VERSION_MINOR( loaded_version ) };
		const uint32_t loader_patch { VK_VERSION_PATCH( loaded_version ) };
		const uint32_t target_major { VK_VERSION_MAJOR( apiVersion ) };
		const uint32_t target_minor { VK_VERSION_MINOR( apiVersion ) };
		const uint32_t target_patch { VK_VERSION_PATCH( apiVersion ) };
		const auto& properties { physical_device.getProperties() };
		std::cout
			<< "\n\tDevice Name: " << properties.deviceName
			<< "\n\tMinimum required Vulkan API v"
			<< target_major << '.' << target_minor << '.' << target_patch
			<< "\n\tDetected running Vulkan API v"
			<< loader_major << '.' << loader_minor << '.' << loader_patch
			<< "\n\tHas support for  Vulkan API v"
			<< VK_VERSION_MAJOR( properties.apiVersion ) << '.'
			<< VK_VERSION_MINOR( properties.apiVersion ) << '.'
			<< VK_VERSION_PATCH( properties.apiVersion )
			<< "\n\tMax Compute Shared Memory Size: "
			<< properties.limits.maxComputeSharedMemorySize / 1024 << " KB"
			<< "\n\tCompute Queue Family Index: "
			<< queue_family_index
			<< "\n\tMax Compute Work Group Sizes: ";

		for( uint32_t index { 0 };
			const auto n : properties.limits.maxComputeWorkGroupSize )
		{
			std::cout << " [" << index++ << "]=" << n;
		}

		std::cout << "\n\tMax compute Work Group Count: ";
		for( uint32_t index { 0 };
			const auto n : properties.limits.maxComputeWorkGroupCount )
		{
			std::cout << " [" << index++ << "]=" << n;
		}

		std::cout << "\n\tMax Compute Inovactions: " << properties.limits.maxComputeWorkGroupInvocations << std::endl;

		std::cout << "\n" << std::endl;
	}
}
/// </DEBUG PRINTING>
#include <unordered_map>

int main() try
{

	stopwatch::Stopwatch mainwatch( "Main" );
	mainwatch.start();

	fgl::vulkan::AppInfo info(
		VK_API_VERSION_1_2,
		{ "VK_LAYER_KHRONOS_validation" },
		{},
		1,
		0.0
	);

	fgl::vulkan::Context inst( info );

	debug_print( inst.context, inst.physical_device, inst.queue_family_index );

	constexpr size_t elements = 512;
	vk::DeviceSize insize = elements * sizeof( uint32_t ) + sizeof( uint32_t );
	vk::DeviceSize outsize = ( elements * elements ) * sizeof( uint32_t );


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


	fgl::vulkan::Pipeline vpipeline( inst, std::filesystem::path( "/home/kj16609/Desktop/Projects/testing/Square.spv" ), std::string( "main" ), buffers );

	{
		void* test = buffers.at( 0 ).get_memory();

		// TODO range wrapper
		uint32_t* const in_buffer_data {
				reinterpret_cast< uint32_t* > ( test )
		};


		in_buffer_data[0] = elements - 1;

		for( size_t i = 1; i < elements; ++i )
		{
			in_buffer_data[i] = static_cast< uint32_t >( i );
		}

		std::cout << "Input Buffer:" << std::endl;
		for( size_t i = 0; i < elements; ++i )
		{
			std::cout << std::setw( 5 ) << in_buffer_data[i] << " ";
		}
		std::cout << std::endl;
		/// PRINT

		constexpr vk::DeviceSize io_buffer_bind_offset { 0 };
		buffers.at( 0 ).memory.unmapMemory();
	}


	const auto command_pool { create_command_pool( inst.device, inst.queue_family_index ) };

	const auto command_buffer { create_command_buffer( inst.device, command_pool ) };

	const vk::CommandBufferBeginInfo bi { vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
	command_buffer.begin( bi );
	command_buffer.bindPipeline( vk::PipelineBindPoint::eCompute, *vpipeline.pipeline );

	std::vector<vk::DescriptorSet> array;


	for( const auto& tlayout : vpipeline.sets )
	{
		array.push_back( *tlayout );
	}

	command_buffer.bindDescriptorSets( vk::PipelineBindPoint::eCompute, *vpipeline.layout, 0, array, nullptr );
	command_buffer.dispatch( elements, elements, 1 );
	command_buffer.end();


	const auto fence { create_waitable_fence( inst.device, command_buffer, inst.queue_family_index ) };
	constexpr uint64_t timeout { 10 };
	while( vk::Result::eTimeout == inst.device.waitForFences( { *fence }, VK_TRUE, timeout ) );

	constexpr vk::DeviceSize out_map_offset { 0 };
	auto out_buffer_ptr = reinterpret_cast< uint32_t* >( buffers.at( 1 ).get_memory() );

	/// PRINT
	std::cout << "Output Buffer:" << std::endl;
	for( size_t y = 0; y < elements; ++y )// spammy...
	{
		for( size_t x = 0; x < elements; ++x )
	{
			auto index = y * elements + x;
			std::cout << std::setw( 5 ) << out_buffer_ptr[index];
		}
		std::cout << std::endl;
	}

	//
	/// PRINT
	buffers.at( 1 ).memory.unmapMemory();

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
