
#include <cstdint>
#include <vector>
#include <random>
#include <bitset>
#include <thread>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <limits>

#include <vulkan/vulkan.hpp>
#include "stopwatch.hpp"

std::random_device rd;
std::mt19937_64 e { rd() };


#define VK_CHECK(x) 													\
do																		\
{																		\
	VkResult err = x;													\
	if( err )															\
	{																	\
		std::cout << "Vulkan FUCKING DIED: " << err << std::endl;		\
		abort();														\
	}																	\
} while( 0 )															



int main()
{
	stopwatch::Stopwatch mainwatch( "Main" );
	mainwatch.start();


	//Application Creation
	vk::ApplicationInfo AppInfo;
	AppInfo.setPApplicationName( "VulkanCompute" );
	AppInfo.setApplicationVersion( 1 );
	AppInfo.setPEngineName( nullptr );
	AppInfo.setEngineVersion( 0 );
	AppInfo.setApiVersion( VK_API_VERSION_1_2 );

	//Instance creation VK_LAYER_KHRONOS_validation
	//const std::vector<const char*> Layers = { "VK_LAYER_KHRONOS_validation" };
	vk::InstanceCreateInfo InstanceCreateInfo;
	InstanceCreateInfo.setFlags( vk::InstanceCreateFlags() );
	InstanceCreateInfo.pApplicationInfo = &AppInfo;
	//InstanceCreateInfo.setPpEnabledLayerNames( Layers.data() );
	//InstanceCreateInfo.setEnabledLayerCount( Layers.size() );


	vk::Instance instance = vk::createInstance( InstanceCreateInfo );

	//Physical Device (GPU)
	vk::PhysicalDevice physicalDevice = instance.enumeratePhysicalDevices().front();
	vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
	std::cout << "Device Name: " << deviceProperties.deviceName << std::endl;
	const uint32_t apiVersion = deviceProperties.apiVersion;
	std::cout << "Vulkan version : " << VK_VERSION_MAJOR( apiVersion ) << "." << VK_VERSION_MINOR( apiVersion ) << "." << VK_VERSION_PATCH( apiVersion ) << std::endl;
	vk::PhysicalDeviceLimits deviceLimits = physicalDevice.getProperties().limits;
	uint32_t sizeLimits = deviceLimits.maxComputeSharedMemorySize / 1024;
	std::cout << "Max Compute Shared Memory Size: " << sizeLimits << " KB" << std::endl;

	//QUEUE
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	auto PropIter = std::find_if( queueFamilyProperties.begin(), queueFamilyProperties.end(), []( const vk::QueueFamilyProperties& prop )
		{
			return prop.queueFlags & vk::QueueFlagBits::eCompute;
		} );

	const uint32_t computeFamilyQueueIndex = std::distance( queueFamilyProperties.begin(), PropIter );
	std::cout << "Compute Queue Family Index: " << computeFamilyQueueIndex << std::endl;

	//Create queue instance
	float queuePrioirty = 1.0f;
	vk::DeviceQueueCreateInfo DeviceQueueCreateInfo;
	DeviceQueueCreateInfo.setQueueCount( 1 );
	DeviceQueueCreateInfo.setQueueFamilyIndex( computeFamilyQueueIndex );
	DeviceQueueCreateInfo.setPQueuePriorities( &queuePrioirty );


	//Device
	vk::DeviceCreateInfo DeviceCreateInfo;
	DeviceCreateInfo.setFlags( vk::DeviceCreateFlags() );
	DeviceCreateInfo.setQueueCreateInfos( DeviceQueueCreateInfo );

	vk::Device device = physicalDevice.createDevice( DeviceCreateInfo );

	uint32_t maxComputeWorkGroupSize = deviceLimits.maxComputeWorkGroupSize[0];
	std::cout << "Max Compute Work Group Size: " << maxComputeWorkGroupSize << std::endl;


	//Buffers

	//Sizes

	constexpr uint32_t matrixSize = 1024 * 8; // n by n matrix
	constexpr uint32_t matrixByteSize = matrixSize * sizeof( uint32_t );
	constexpr uint32_t matrixSizeSquared = matrixSize * matrixSize;
	constexpr uint32_t matrixSquaredByteSize = matrixSizeSquared * sizeof( float );

	std::cout << "Matrix Size: " << matrixSize << std::endl;
	std::cout << "Matrix Byte Size: " << matrixByteSize << std::endl;
	std::cout << "Matrix Size Squared: " << matrixSizeSquared << std::endl;
	std::cout << "Matrix Squared Byte Size: " << matrixSquaredByteSize << std::endl;



	vk::BufferCreateInfo inputBufferInfo;
	inputBufferInfo.setFlags( vk::BufferCreateFlags() );
	inputBufferInfo.setSize( matrixByteSize + ( sizeof( uint32_t ) * 2 ) );
	inputBufferInfo.setUsage( vk::BufferUsageFlagBits::eStorageBuffer );
	inputBufferInfo.setSharingMode( vk::SharingMode::eExclusive );
	inputBufferInfo.setQueueFamilyIndexCount( 1 );
	inputBufferInfo.setPQueueFamilyIndices( &computeFamilyQueueIndex );

	vk::BufferCreateInfo outputBufferInfo;
	outputBufferInfo.setFlags( vk::BufferCreateFlags() );
	outputBufferInfo.setSize( matrixSquaredByteSize );
	outputBufferInfo.setUsage( vk::BufferUsageFlagBits::eStorageBuffer );
	outputBufferInfo.setSharingMode( vk::SharingMode::eExclusive );
	outputBufferInfo.setQueueFamilyIndexCount( 1 );
	outputBufferInfo.setPQueueFamilyIndices( &computeFamilyQueueIndex );

	//Buffer creation
	vk::Buffer inputBuffer = device.createBuffer( inputBufferInfo );
	vk::Buffer outputBuffer = device.createBuffer( outputBufferInfo );

	//Memory
	vk::MemoryRequirements inputMemoryRequirements = device.getBufferMemoryRequirements( inputBuffer );
	vk::MemoryRequirements outputMemoryRequirements = device.getBufferMemoryRequirements( outputBuffer );

	vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

	uint32_t memoryTypeIndex = uint32_t( std::numeric_limits<uint32_t>::max() );
	vk::DeviceSize memoryHeapSize = uint32_t( std::numeric_limits<uint32_t>::max() );

	for( uint32_t currentMemoryTypeIndex = 0; currentMemoryTypeIndex < memoryProperties.memoryTypeCount; ++currentMemoryTypeIndex )
	{
		vk::MemoryType memoryType = memoryProperties.memoryTypes[currentMemoryTypeIndex];
		if( ( vk::MemoryPropertyFlagBits::eHostVisible & memoryType.propertyFlags ) && ( vk::MemoryPropertyFlagBits::eHostCoherent & memoryType.propertyFlags ) )
		{
			memoryHeapSize = memoryProperties.memoryHeaps[memoryType.heapIndex].size;
			memoryTypeIndex = currentMemoryTypeIndex;
			break;
		}
	}

	std::cout << "Memory Type Index: " << memoryTypeIndex << std::endl;
	std::cout << "Memory Heap Size: " << memoryHeapSize << std::endl;

	vk::MemoryAllocateInfo inMemoryAllocateInfo;
	inMemoryAllocateInfo.setAllocationSize( inputMemoryRequirements.size );
	inMemoryAllocateInfo.setMemoryTypeIndex( memoryTypeIndex );

	vk::MemoryAllocateInfo outMemoryAllocateInfo;
	outMemoryAllocateInfo.setAllocationSize( outputMemoryRequirements.size );
	outMemoryAllocateInfo.setMemoryTypeIndex( memoryTypeIndex );

	vk::DeviceMemory inputMemory = device.allocateMemory( inMemoryAllocateInfo );
	vk::DeviceMemory outputMemory = device.allocateMemory( outMemoryAllocateInfo );

	std::cout << "Input Requirements: " << inputMemoryRequirements.size << std::endl;
	std::cout << "Output Requirements: " << outputMemoryRequirements.size << std::endl;

	int32_t* inBufferData = static_cast< int32_t* >( device.mapMemory( inputMemory, 0, matrixByteSize ) );

	//Populate buffer
	inBufferData[0] = matrixSize;
	for( uint32_t i = 0; i < matrixSize + 2; ++i )
	{
		inBufferData[i + 1] = i;
	}

	for( uint32_t i = 0; i < matrixSize + 2; ++i )
	{
		//std::cout << inBufferData[i] << " ";
	}

	device.unmapMemory( inputMemory );

	//binding to memory
	device.bindBufferMemory( inputBuffer, inputMemory, 0 );
	device.bindBufferMemory( outputBuffer, outputMemory, 0 );

	//shader
	std::ifstream file { "./src/Square.sqv", std::ios::ate | std::ios::binary };
	if( !file.is_open() )
	{
		std::cout << "Failed to open file" << std::endl;
		return 1;
	}
	uint32_t fileSize = file.tellg();
	file.seekg( 0 );
	std::vector<char> shaderCode( fileSize );
	file.read( shaderCode.data(), fileSize );
	file.close();


	vk::ShaderModuleCreateInfo shaderModuleCreateInfo;
	shaderModuleCreateInfo.setFlags( vk::ShaderModuleCreateFlags() );
	shaderModuleCreateInfo.setCodeSize( shaderCode.size() );
	shaderModuleCreateInfo.setPCode( reinterpret_cast< const uint32_t* >( shaderCode.data() ) );

	vk::ShaderModule shaderModule = device.createShaderModule( shaderModuleCreateInfo );

	//Descirptor
	const std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
		vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute ),
		vk::DescriptorSetLayoutBinding( 1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute )
	};

	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	descriptorSetLayoutCreateInfo.setFlags( vk::DescriptorSetLayoutCreateFlags() );
	descriptorSetLayoutCreateInfo.setBindingCount( descriptorSetLayoutBindings.size() );
	descriptorSetLayoutCreateInfo.setPBindings( descriptorSetLayoutBindings.data() );

	vk::DescriptorSetLayout descriptorSetLayout = device.createDescriptorSetLayout( descriptorSetLayoutCreateInfo );

	//PIPELINE
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.setFlags( vk::PipelineLayoutCreateFlags() );
	pipelineLayoutCreateInfo.setSetLayoutCount( 1 );
	pipelineLayoutCreateInfo.setPSetLayouts( &descriptorSetLayout );

	vk::PipelineLayout pipelineLayout = device.createPipelineLayout( pipelineLayoutCreateInfo );
	vk::PipelineCache pipelineCache = device.createPipelineCache( vk::PipelineCacheCreateInfo() );

	//pipeline
	vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo;
	pipelineShaderStageCreateInfo.setFlags( vk::PipelineShaderStageCreateFlags() );
	pipelineShaderStageCreateInfo.setStage( vk::ShaderStageFlagBits::eCompute );
	pipelineShaderStageCreateInfo.setModule( shaderModule );
	pipelineShaderStageCreateInfo.setPName( "main" );

	vk::ComputePipelineCreateInfo computePipelineCreateInfo;
	computePipelineCreateInfo.setFlags( vk::PipelineCreateFlags() );
	computePipelineCreateInfo.setStage( pipelineShaderStageCreateInfo );
	computePipelineCreateInfo.setLayout( pipelineLayout );

	vk::Pipeline computePipeline = device.createComputePipeline( pipelineCache, computePipelineCreateInfo ).value;

	//Descriptor
	vk::DescriptorPoolSize descriptorPoolSize;
	descriptorPoolSize.setType( vk::DescriptorType::eStorageBuffer );
	descriptorPoolSize.setDescriptorCount( 2 );

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.setFlags( vk::DescriptorPoolCreateFlags() );
	descriptorPoolCreateInfo.setMaxSets( 1 );
	descriptorPoolCreateInfo.setPoolSizeCount( 1 );
	descriptorPoolCreateInfo.setPPoolSizes( &descriptorPoolSize );

	vk::DescriptorPool descriptorPool = device.createDescriptorPool( descriptorPoolCreateInfo );

	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo;
	descriptorSetAllocateInfo.setDescriptorPool( descriptorPool );
	descriptorSetAllocateInfo.setDescriptorSetCount( 1 );
	descriptorSetAllocateInfo.setPSetLayouts( &descriptorSetLayout );

	const std::vector<vk::DescriptorSet> descriptorSets = device.allocateDescriptorSets( descriptorSetAllocateInfo );

	vk::DescriptorSet descriptorSet = descriptorSets.front();

	vk::DescriptorBufferInfo InBufferInfo;
	InBufferInfo.setBuffer( inputBuffer );
	InBufferInfo.setOffset( 0 );
	InBufferInfo.setRange( matrixByteSize );

	vk::DescriptorBufferInfo OutBufferInfo;
	OutBufferInfo.setBuffer( outputBuffer );
	OutBufferInfo.setOffset( 0 );
	OutBufferInfo.setRange( matrixSquaredByteSize );

	std::vector<vk::WriteDescriptorSet> writeDescriptorSets = {
		vk::WriteDescriptorSet( descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &InBufferInfo ),
		vk::WriteDescriptorSet( descriptorSet, 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &OutBufferInfo )
	};

	device.updateDescriptorSets( writeDescriptorSets, nullptr );

	//command buffer
	vk::CommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.setFlags( vk::CommandPoolCreateFlags() );
	commandPoolCreateInfo.setQueueFamilyIndex( computeFamilyQueueIndex );

	vk::CommandPool commandPool = device.createCommandPool( commandPoolCreateInfo );

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.setCommandPool( commandPool );
	commandBufferAllocateInfo.setLevel( vk::CommandBufferLevel::ePrimary );
	commandBufferAllocateInfo.setCommandBufferCount( 1 );

	const std::vector<vk::CommandBuffer> commandBuffers = device.allocateCommandBuffers( commandBufferAllocateInfo );
	vk::CommandBuffer commandBuffer = commandBuffers.front();

	vk::CommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.setFlags( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

	std::cout << "Begin Command Buffer" << std::endl;
	stopwatch::Stopwatch commandWatch( "Command Queue" );
	commandWatch.start();
	commandBuffer.begin( commandBufferBeginInfo );
	commandBuffer.bindPipeline( vk::PipelineBindPoint::eCompute, computePipeline );
	commandBuffer.bindDescriptorSets( vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSet, nullptr );

	//Calculating matrix
	std::cout << "Dispatching compute shader" << std::endl;
	//commandBuffer.dispatch( ( matrixSizeSquared / 1 ) + 1, 1, 1 );
	commandBuffer.dispatch( 1, 1, 1 );
	commandBuffer.end();

	vk::Queue queue = device.getQueue( computeFamilyQueueIndex, 0 );
	vk::Fence fence = device.createFence( vk::FenceCreateInfo() );
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBufferCount( 1 );
	submitInfo.setPCommandBuffers( &commandBuffer );

	queue.submit( submitInfo, fence );
	device.waitForFences( { fence }, VK_TRUE, UINT64_MAX );
	commandWatch.stop();
	//Get result
	int32_t* outBufferPtr = static_cast< int32_t* >( device.mapMemory( outputMemory, 0, matrixSquaredByteSize ) );

	/*
		std::cout << std::endl;
		for( int i = 0; i < matrixSizeSquared; i++ )
		{
			std::cout << std::setw( 5 ) << outBufferPtr[i] << " ";
			if( ( i + 1 ) % matrixSize == 0 )
			{
				std::cout << std::endl;
			}
		}*/



	stopwatch::Stopwatch cpuTimer( "CPU" );
	inBufferData = new int32_t[matrixSize];
	int32_t* outBufferData = new int32_t[matrixSizeSquared];

	for( uint32_t i = 0; i < matrixSize; ++i )
	{
		inBufferData[i] = i;
	}

	cpuTimer.start();

	for( size_t i = 0; i < matrixSize; i++ )
	{
		for( size_t j = 0; j < matrixSize; j++ )
		{
			outBufferData[i * matrixSize + j] = exp( sqrt( inBufferData[i] * inBufferData[j] ) + exp( inBufferData[i] ) * inBufferData[j] );
		}
	}

	cpuTimer.stop();



	delete[] inBufferData;




	std::cout << std::endl;
	device.unmapMemory( outputMemory );

	//CLEANUP
	device.resetCommandPool( commandPool );
	device.destroyFence( fence );
	device.destroyDescriptorSetLayout( descriptorSetLayout );
	device.destroyPipelineLayout( pipelineLayout );
	device.destroyPipelineCache( pipelineCache );
	device.destroyShaderModule( shaderModule );
	device.destroyPipeline( computePipeline );
	device.destroyDescriptorPool( descriptorPool );
	device.destroyCommandPool( commandPool );
	device.destroyBuffer( inputBuffer );
	device.destroyBuffer( outputBuffer );
	device.freeMemory( inputMemory );
	device.freeMemory( outputMemory );
	device.destroy();
	instance.destroy();
	//vkCmdDispatch( cmdbuffer, numOfMembers / 8 + 1, numOfMembers / 8 + 1, 1 );





	mainwatch.stop();
	std::cout << mainwatch << std::endl;
	std::cout << cpuTimer << std::endl;
	std::cout << commandWatch << std::endl;
}
