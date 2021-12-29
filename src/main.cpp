#include <cstdlib> // abort, EXIT_SUCCESS
#include <cstdint>
#include <array>
#include <utility> // move, pair
#include <algorithm> // find_if
#include <sstream> // used for API version exception
#include <filesystem>
#include <fstream>


#include <string_view>
#include <iostream> // cout, cerr, endl

#include <vulkan/vulkan_raii.hpp>

#include "stopwatch.hpp"

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

auto create_instance( const vk::raii::Context& context )
{
	constexpr vk::ApplicationInfo appInfo(
		appName, appVersion, engineName, engineVersion, apiVersion
	);
	const vk::InstanceCreateInfo ci(
		vk::InstanceCreateFlags {}, &appInfo,
		layers.size(), layers.data(),
		extensions.size(), extensions.data()
	);

	return vk::raii::Instance( context, ci );
}

//returns the index to the first queueFamiliyProperties with flag set.
uint32_t index_of_first_queue_family(
	vk::QueueFlagBits flag,
	const std::vector<vk::QueueFamilyProperties>& properties )
{
	const auto has_flag {
		[flag]( const vk::QueueFamilyProperties& qfp ) noexcept -> bool
		{ // lambda function for find_if predicate
 			return static_cast< bool >( qfp.queueFlags & flag );
		}
	};
	if( const auto it { std::ranges::find_if( properties, has_flag ) };
		it != properties.cend() )
	{
		return static_cast< uint32_t >( std::distance( properties.cbegin(), it ) );
	}
	else throw std::runtime_error(
		"Vulkan couldn't find a graphics queue family."
	);
}

auto create_device(
	const vk::raii::PhysicalDevice& physical_device,
	const uint32_t queue_family_index )
{
	constexpr float queue_count { 1 };
	constexpr float queue_priority { 0.0f };
	const vk::DeviceQueueCreateInfo device_queue_ci(
		{}, queue_family_index, queue_count, &queue_priority
	);
	const vk::DeviceCreateInfo device_ci( {}, device_queue_ci );

	return vk::raii::Device( physical_device, device_ci );
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

auto create_buffer(
	const vk::raii::Device& device,
	const uint32_t size,
	const uint32_t queue_family_index )
{
	constexpr uint32_t number_of_family_indexes { 1 };
	const vk::BufferCreateInfo ci(
		{},
		size,
		vk::BufferUsageFlagBits::eStorageBuffer,
		vk::SharingMode::eExclusive,
		number_of_family_indexes,
		&queue_family_index
	);
	return device.createBuffer( ci );
}

std::pair<uint32_t, vk::DeviceSize> get_memory_type(
	const vk::MemoryPropertyFlags& flags,
	const vk::PhysicalDeviceMemoryProperties& device_memory_properties )
{
	for(
		uint32_t current_memory_index = 0;
		current_memory_index < device_memory_properties.memoryTypeCount;
		++current_memory_index )
	{
		const vk::MemoryType memoryType{
			device_memory_properties.memoryTypes[current_memory_index]
		};
		if( flags & memoryType.propertyFlags )
		{
			const vk::DeviceSize memory_heap_size{
				 device_memory_properties.memoryHeaps[memoryType.heapIndex].size
			};
			return std::pair { current_memory_index, memory_heap_size };
		}
	}
	throw std::runtime_error( "Failed to get memory type." );
}

auto create_device_memory(
	const vk::raii::Device& device,
	const vk::raii::PhysicalDevice& physical_device,
	const vk::raii::Buffer& buffer,
	const vk::MemoryPropertyFlags& flags)
{
	const auto [index, size] { get_memory_type( flags, physical_device.getMemoryProperties() ) };
	const vk::MemoryAllocateInfo memInfo( buffer.getMemoryRequirements().size, index );

	return device.allocateMemory( memInfo );
}

vk::raii::ShaderModule create_shader_module(
	const vk::raii::Device& device,
	const std::vector<char>& data )
{
	const vk::ShaderModuleCreateInfo ci(
		vk::ShaderModuleCreateFlags(),
		data.size(),
		reinterpret_cast< const uint32_t* >(
			reinterpret_cast<const void*>(data.data())
		)
	);
	return vk::raii::ShaderModule( device, ci );
}

vk::raii::ShaderModule create_shader_module_from_file(
	const vk::raii::Device& device,
	const std::filesystem::path& path )
{
	std::vector<char> buf;

	if( std::ifstream ifs( path.string(), std::ios::binary | std::ios::ate );
		ifs )
	{
		ifs.exceptions( ifs.badbit | ifs.failbit | ifs.eofbit );
		buf.resize( static_cast< uint32_t >( ifs.tellg() ) );
		ifs.seekg( 0 );
		ifs.read( buf.data(), static_cast<std::streamsize>(buf.size()) );
	}
	else
	{
		throw std::runtime_error( "Failed to open shader file: " + path.string() );
	}

	return create_shader_module( device, buf );
}

vk::raii::DescriptorSetLayout create_descriptor_set_layout(
	const vk::raii::Device& device,
	const std::vector<vk::DescriptorSetLayoutBinding>& bindings )
{
	const vk::DescriptorSetLayoutCreateInfo ci(
		{},
		static_cast<uint32_t>(bindings.size()),
		bindings.data()
	);
	return vk::raii::DescriptorSetLayout( device, ci );
}

vk::raii::PipelineLayout create_pipeline_layout(
	const vk::raii::Device& device,
	const vk::raii::DescriptorSetLayout& descriptor_set_layout )
{
	const vk::PipelineLayoutCreateInfo ci( {}, *descriptor_set_layout );
	return vk::raii::PipelineLayout( device, ci );
}


vk::raii::Pipeline create_pipeline(
	const vk::raii::Device& device,
	const vk::raii::PipelineLayout& pipeline_layout,
	const vk::raii::ShaderModule& shader,
	const std::string& init_function_name,
	const vk::ShaderStageFlagBits stage )
{
	const vk::PipelineShaderStageCreateInfo shader_stage_info(
		{},
		stage,
		*shader,
		init_function_name.c_str()
	);

	switch( stage )
	{
	using enum vk::ShaderStageFlagBits;
	break;case eCompute:
	{
		const vk::ComputePipelineCreateInfo ci(
			{},
			shader_stage_info,
			*pipeline_layout
		);
		return device.createComputePipeline(
			device.createPipelineCache( vk::PipelineCacheCreateInfo() ),
			ci
		);
	}
	break;case eTessellationControl:
	break;case eTessellationEvaluation:
	break;case eGeometry:
	break;case eFragment:
	break;case eAllGraphics:
	break;case eAll:
	break;case eRaygenKHR: // eRaygenNV
	break;case eAnyHitKHR:
	break;case eClosestHitKHR: // eClosestHitNV
	break;case eMissKHR: // eMissNV
	break;case eIntersectionKHR: // eIntersectionNV
	break;case eCallableKHR: // eCallableNV
	break;case eTaskNV:
	break;case eMeshNV:
	break;case eSubpassShadingHUAWEI:
	break;case eVertex:
	break;default:break;
	}
	throw std::runtime_error( "Unsupported shader stage" );
}

vk::raii::DescriptorPool create_descriptor_pool(
	const vk::raii::Device& device,
	const vk::DescriptorType type,
	const uint32_t DescriptorCount )
{
	constexpr uint32_t max_sets { 1 };
	constexpr uint32_t pool_size_count { 1 };
	const vk::DescriptorPoolSize poolSize { type, DescriptorCount };
	const vk::DescriptorPoolCreateInfo poolInfo {
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		max_sets,
		pool_size_count,
		&poolSize
	};
	return vk::raii::DescriptorPool( device, poolInfo );
}

vk::raii::DescriptorSets create_descriptor_sets(
	const vk::raii::Device& device,
	const vk::raii::DescriptorPool& pool,
	const vk::raii::DescriptorSetLayout& layout )
{
	const vk::DescriptorSetAllocateInfo ci( *pool, *layout );
	return vk::raii::DescriptorSets( device, ci );
}

std::pair<vk::WriteDescriptorSet, std::shared_ptr<vk::DescriptorBufferInfo>> create_descriptor_set(
	const vk::raii::Buffer& buffer,
	const uint32_t offset,
	const uint32_t size,
	const vk::raii::DescriptorSet& descriptor_set,
	const uint32_t binding,
	const uint32_t array,
	const uint32_t descriptorCount,
	const vk::DescriptorType type )
{
	std::shared_ptr<vk::DescriptorBufferInfo> bi = std::make_shared<vk::DescriptorBufferInfo>( *buffer, offset, size );

	constexpr vk::DescriptorImageInfo* imagepointer { nullptr };

	std::cout << "Buffer descriptor set made" << std::endl;
	;
	return std::pair( vk::WriteDescriptorSet( *descriptor_set, binding, array, descriptorCount, type, imagepointer, bi.get() ), bi );
}

void update_descriptor_set( vk::raii::Device& device, std::vector < std::pair<vk::WriteDescriptorSet, std::shared_ptr<vk::DescriptorBufferInfo>>>& set )
{
	//remake vec
	std::vector<vk::WriteDescriptorSet> remade_set;
	for( auto& pair : set )
	{
		remade_set.push_back( pair.first );
	}

	device.updateDescriptorSets( remade_set, nullptr );
}

void dispatch_command_buffer(
	const vk::raii::CommandBuffer& command_buffer,
	const vk::CommandBufferUsageFlagBits buffer_usage_flags,
	const vk::raii::Pipeline& pipeline,
	const vk::raii::PipelineLayout& pipeline_layout,
	const vk::raii::DescriptorSet& descriptor_set,
	const vk::PipelineBindPoint bind_point,
	const uint32_t dispatch_x,
	const uint32_t dispatch_y, // TODO vec3?
	const uint32_t dispatch_z)
{
	const vk::CommandBufferBeginInfo bi { buffer_usage_flags };
	command_buffer.begin( bi );
	command_buffer.bindPipeline( bind_point , *pipeline );
	command_buffer.bindDescriptorSets( bind_point, *pipeline_layout, 0, *descriptor_set, nullptr );
	command_buffer.dispatch( dispatch_x, dispatch_y, dispatch_z );
	command_buffer.end();
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

int main() try
{
	stopwatch::Stopwatch mainwatch( "Main" );
	mainwatch.start();

	vk::raii::Context context;
	verify_version_requirements( context );
	vk::raii::Instance instance { create_instance( context ) };
	vk::raii::PhysicalDevice physical_device { std::move( vk::raii::PhysicalDevices( instance ).front() ) };

	const uint32_t queue_family_index {
		index_of_first_queue_family(
			vk::QueueFlagBits::eCompute,
			physical_device.getQueueFamilyProperties() )
	};

	debug_print( context, physical_device, queue_family_index );

	auto device { create_device( physical_device, queue_family_index ) };
	auto command_pool { create_command_pool( device, queue_family_index ) };
	auto command_buffer { create_command_buffer( device, command_pool ) };

	constexpr uint32_t num_of_members { 512 };
	constexpr uint32_t blocksize { num_of_members * sizeof( uint32_t ) };
	constexpr uint32_t in_size { blocksize + sizeof( uint32_t ) * 2 };
	constexpr uint32_t out_size { blocksize * blocksize };

	auto input_buffer { create_buffer( device, in_size, queue_family_index ) };
	auto output_buffer { create_buffer( device, out_size, queue_family_index ) };

	//Memory

	constexpr auto memory_flags { vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	auto input_memory { create_device_memory( device, physical_device, input_buffer, memory_flags ) };
	auto output_memory { create_device_memory( device, physical_device, output_buffer, memory_flags ) };

	// TODO range wrapper
	uint32_t* in_buffer_data = static_cast< uint32_t* > ( input_memory.mapMemory( 0, sizeof( uint32_t ) * ( num_of_members + 2 ) ) );

	constexpr uint32_t offset = 1;

	in_buffer_data[0] = num_of_members;
	for( uint32_t i = 0 + offset; i < num_of_members + offset; ++i )
	{
		in_buffer_data[i] = i;
	}

	std::cout << "Input Buffer:" << std::endl;

	for( size_t i = 0; i < num_of_members + offset; ++i )
	{
		std::cout << std::setw( 5 ) << in_buffer_data[i] << " ";
	}
	std::cout << std::endl;

	constexpr vk::DeviceSize io_buffer_bind_offset { 0 };
	input_memory.unmapMemory();
	input_buffer.bindMemory( *input_memory, io_buffer_bind_offset );
	output_buffer.bindMemory( *output_memory, io_buffer_bind_offset );

	//Loading shader
	vk::raii::ShaderModule module1{ create_shader_module_from_file( device, "./Square.spv" ) };

	//Discriptors
	constexpr uint32_t descriptor_count { 1 };
	const std::vector<vk::DescriptorSetLayoutBinding> descriptorSetBindings =
	{
		vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eStorageBuffer, descriptor_count, vk::ShaderStageFlagBits::eCompute ),
		vk::DescriptorSetLayoutBinding( 1, vk::DescriptorType::eStorageBuffer, descriptor_count, vk::ShaderStageFlagBits::eCompute )
	};


	vk::raii::DescriptorSetLayout descriptor_set_layout = create_descriptor_set_layout( device, descriptorSetBindings );

	//Pipeline
	vk::raii::PipelineLayout pipeline_layout = create_pipeline_layout( device, descriptor_set_layout );

	vk::raii::Pipeline pipeline = create_pipeline( device, pipeline_layout, module1, "main", vk::ShaderStageFlagBits::eCompute );

	vk::raii::DescriptorPool descriptor_pool = create_descriptor_pool( device, vk::DescriptorType::eStorageBuffer, 2 );

	vk::raii::DescriptorSets descriptor_sets = create_descriptor_sets( device, descriptor_pool, descriptor_set_layout );

	vk::raii::DescriptorSet descriptor_set = std::move( descriptor_sets.front() );

	//vk::DescriptorBufferInfo bi1( *input_buffer, 0, in_size );

	std::vector set =
	{
		create_descriptor_set( input_buffer, 0, in_size, descriptor_set, 0, 0, 1, vk::DescriptorType::eStorageBuffer ),
		create_descriptor_set( output_buffer, 0, out_size, descriptor_set, 1, 0, 1, vk::DescriptorType::eStorageBuffer )
	};

	update_descriptor_set( device, set );

	dispatch_command_buffer(
		command_buffer,
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
		pipeline,
		pipeline_layout,
		descriptor_set,
		vk::PipelineBindPoint::eCompute,
		num_of_members / 4,
		num_of_members / 4,
		1
	);
	vk::raii::Queue queue = device.getQueue( queue_family_index, 0 );
	vk::raii::Fence fence = device.createFence( vk::FenceCreateInfo() );

	const vk::SubmitInfo submit_info( nullptr, nullptr, *command_buffer, nullptr );
	queue.submit( submit_info, *fence );

	while( vk::Result::eTimeout == device.waitForFences( { *fence }, VK_TRUE, 10 ) );
	[[maybe_unused]] int32_t* out_buffer_ptr = static_cast< int32_t* >( output_memory.mapMemory( 0, out_size ) );
	/*
	std::cout << "Output Buffer:" << std::endl;
	for( size_t i = 0; i < num_of_members * num_of_members; ++i )
	{
		std::cout << std::setw( 5 ) << out_buffer_ptr[i] << " ";
		if( ( i + 1 ) % num_of_members == 0 )
		{
			std::cout << std::endl;
		}
	}
	//*/

	output_memory.unmapMemory();

	mainwatch.stop();
	std::cout << mainwatch << std::endl;
}
catch( const vk::SystemError& e )
{
	std::cerr << "\n\n Vulkan system error:\n\t" << e.what() << std::endl;
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
