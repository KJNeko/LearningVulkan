#include <vulkan/vulkan_raii.hpp>

#include <fgl/vulkan/context.hpp>

namespace fgl::vulkan
{
	uint32_t Context::index_of_first_queue_family(const vk::QueueFlagBits flag ) const
	{
		const auto has_flag {
			[flag]( const vk::QueueFamilyProperties& qfp ) noexcept -> bool
			{ // lambda function for find_if predicate
				return static_cast< bool >( qfp.queueFlags & flag );
			}
		};

		const auto end = physical_device.getQueueFamilyProperties().cend();
		const std::vector<vk::QueueFamilyProperties>& props {
			physical_device.getQueueFamilyProperties()
		};

		if( const auto it { std::ranges::find_if( props, has_flag ) };
			it != end )
		{
			return static_cast< uint32_t >( std::distance( props.cbegin(), it ) );
		}
		else throw std::runtime_error(
			"Vulkan couldn't find a graphics queue family."
		);
	}

namespace internal {
	vk::raii::Instance create_instance(
		const vk::raii::Context& context,
		const AppInfo& info )
	{
		const vk::ApplicationInfo appInfo(
			"VulkanCompute", 0, "ComputeEngine", 0, info.apiVersion
		);
		vk::InstanceCreateInfo ci(
			vk::InstanceCreateFlags {}, &appInfo,
			static_cast< uint32_t >( info.layer.size() ), info.layer.data(),
			static_cast< uint32_t >( info.extentions.size() ), info.extentions.data()
		);
		return vk::raii::Instance( context, ci );
	}

	vk::raii::Device create_device(
		const vk::raii::PhysicalDevice& physical_device,
		const uint32_t queue_count,
		const float queue_priority,
		const uint32_t queue_family_index)
	{
		const vk::DeviceQueueCreateInfo device_queue_ci(
			{}, queue_family_index, queue_count, &queue_priority
		);

		std::vector<const char*> layers;
		std::vector<const char*> extentions { };

		const vk::DeviceCreateInfo device_ci( {}, device_queue_ci, layers, extentions );

		return vk::raii::Device( physical_device, device_ci );
	}
} // namespace internal

	Context::Context( const AppInfo& info )
	:
		context{},
		instance( internal::create_instance( context, info ) ),
		physical_device( std::move( vk::raii::PhysicalDevices( instance ).front() ) ),
		queue_family_index( index_of_first_queue_family( vk::QueueFlagBits::eCompute ) ),
		device( internal::create_device(physical_device, info.queue_count, info.queue_priority, queue_family_index) ),
		properties( physical_device.getProperties() ),
		version_info(context.enumerateInstanceVersion(), info.apiVersion)
	{}

	/// INFO PRINTING

	void Context::print_debug_info() const
	{
		const auto& [loaded_version, target_version]{ version_info };
		const internal::Version device_version( properties.apiVersion );

		std::cout
			<< "\n\tDevice Name: " << properties.deviceName
			<< "\n\tMinimum required Vulkan API v" << target_version
			<< "\n\tDetected running Vulkan API v" << loaded_version
			<< "\n\tHas support for  Vulkan API v" << device_version
			<< "\n\tMax Compute Shared Memory Size: "
			<< properties.limits.maxComputeSharedMemorySize / 1024 << " KB"
			<< "\n\tCompute Queue Family Index: "
			<< queue_family_index
			<< std::endl;

		//Checking if we are an AMD gpu
		//FUCK NVIDIA WHY DONT YOU HAVE THIS!?!?!??!!?
		/* what about...
			vk::PhysicalDeviceCooperativeMatrixPropertiesNV
			vk::PhysicalDeviceMeshShaderPropertiesNV
			vk::PhysicalDeviceRayTracingPropertiesNV
			vk::PhysicalDeviceShaderSMBuiltinsPropertiesNV
			vk::PhysicalDeviceShadingRateImagePropertiesNV
		*/
		std::vector<vk::ExtensionProperties> extensionProperties {
			physical_device.enumerateDeviceExtensionProperties()
		};

		const std::string amd_properties_name { "VK_AMD_shader_core_properties2" };
		const bool has_amd_proprties {
			std::find_if(
				extensionProperties.begin(),
				extensionProperties.end(),
				[&amd_properties_name]( vk::ExtensionProperties const& ep )
				{
					return amd_properties_name == ep.extensionName;
				}
			)
			!= extensionProperties.end()
		};

		if( has_amd_proprties )
		{
			const auto properties2 {
				physical_device.getProperties2<
					vk::PhysicalDeviceProperties2,
					vk::PhysicalDeviceShaderCorePropertiesAMD
				>()
			};

			const vk::PhysicalDeviceShaderCorePropertiesAMD& shaderCoreProperties {
				properties2.get<vk::PhysicalDeviceShaderCorePropertiesAMD>()
			};

			std::cout
				<< "\n\tShader Engine Count: " << shaderCoreProperties.shaderEngineCount
				<< "\n\tShader Arrays Per Engine Count: " << shaderCoreProperties.shaderArraysPerEngineCount
				<< "\n\tCompute Units Per Shader Array: " << shaderCoreProperties.computeUnitsPerShaderArray
				<< "\n\tSIMD Per Compute Unit: " << shaderCoreProperties.simdPerComputeUnit
				<< "\n\twavefronts Per SIMD: " << shaderCoreProperties.wavefrontsPerSimd;
		}


		const std::string nv_properties_name { "VK_NV_shader_sm_builtins" };
		const bool has_nv_proprties {
			std::find_if(
				extensionProperties.begin(),
				extensionProperties.end(),
				[&nv_properties_name]( vk::ExtensionProperties const& ep )
				{
					return nv_properties_name == ep.extensionName;
				}
			)
			!= extensionProperties.end()
		};

		if( has_nv_proprties )
		{
			const auto properties2 {
				physical_device.getProperties2<
					vk::PhysicalDeviceProperties2,
					vk::PhysicalDeviceShaderSMBuiltinsPropertiesNV
				>()
			};

			const vk::PhysicalDeviceShaderSMBuiltinsPropertiesNV& shaderCoreProperties {
				properties2.get<vk::PhysicalDeviceShaderSMBuiltinsPropertiesNV>()
			};

			std::cout
				<< "\n\tShader SM Count " << shaderCoreProperties.shaderSMCount
				<< "\n\tShader Warps Per SM " << shaderCoreProperties.shaderWarpsPerSM;
		}

		std::cout << "\n\n\tMax Compute Work Group Sizes: ";

		for( uint32_t index { 0 };
			const auto n : properties.limits.maxComputeWorkGroupSize )
		{
			std::cout << " [" << index++ << "]=" << n;
		}

		std::cout << "\n\tMax Compute Work Group Count: ";
		for( uint32_t index { 0 };
			const auto n : properties.limits.maxComputeWorkGroupCount )
		{
			std::cout << " [" << index++ << "]=" << n;
		}

		std::cout
			<< "\n\tMax Compute Inovactions: "
			<< properties.limits.maxComputeWorkGroupInvocations
			<< "\n\n" << std::endl;
	}
}
