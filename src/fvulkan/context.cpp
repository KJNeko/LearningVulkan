#include <vulkan/vulkan_raii.hpp>

#include <fgl/vulkan/context.hpp>

namespace fgl::vulkan
{
	uint32_t Context::index_of_first_queue_family( const vk::QueueFlagBits flag ) const
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

	namespace internal
	{
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
			const uint32_t queue_family_index )
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
		context {},
		instance( internal::create_instance( context, info ) ),
		physical_device( std::move( vk::raii::PhysicalDevices( instance ).front() ) ),
		queue_family_index( index_of_first_queue_family( vk::QueueFlagBits::eCompute ) ),
		device( internal::create_device( physical_device, info.queue_count, info.queue_priority, queue_family_index ) ),
		properties( physical_device.getProperties() ),
		version_info( context.enumerateInstanceVersion(), info.apiVersion )
	{}

	/// INFO PRINTING

	namespace internal::properties_output
	{

		//AMD
		std::ostream& operator<<(
			std::ostream& os,
			const vk::PhysicalDeviceShaderCoreProperties2AMD& prop
			)
		{
			return os
				<< "\n\tShader Core Properties 2 [AMD]:"
				<< "\n\t\tActive Compute Unit Count: " << prop.activeComputeUnitCount
				<< "\n\t\tShader Core Features: " << vk::to_string( prop.shaderCoreFeatures );
		}

		std::ostream& operator<<(
			std::ostream& os,
			const vk::PhysicalDeviceShaderCorePropertiesAMD& prop )
		{
			return os
				<< "\n\tShader Core Properties [AMD]:"
				<< "\n\t\tShader Engine Count: " << prop.shaderEngineCount
				<< "\n\t\tShader Arrays Per Engine Count: " << prop.shaderArraysPerEngineCount
				<< "\n\t\tCompute Units Per Shader Array: " << prop.computeUnitsPerShaderArray
				<< "\n\t\tSIMD Per Compute Unit: " << prop.simdPerComputeUnit
				<< "\n\t\tWavefronts Per SIMD: " << prop.wavefrontsPerSimd
				<< "\n\t\tWavefronts size: " << prop.wavefrontSize
				<< "\n\n\t\tSgpr Allocation: [MIN]=" << prop.minSgprAllocation
				<< " [MAX]=" << prop.maxSgprAllocation
				<< "\n\t\tSgpr Allocation Granularity: " << prop.sgprAllocationGranularity
				<< "\n\t\tSpgrs Per SIMD: " << prop.sgprsPerSimd
				<< "\n\n\t\tVgpr Allocation: [MIN]=" << prop.minVgprAllocation
				<< " [MIN]=" << prop.maxVgprAllocation
				<< "\n\t\tVgpr Allocation Granularity: " << prop.vgprAllocationGranularity
				<< "\n\t\tVgprs Per SIMD: " << prop.vgprsPerSimd;

		}


		//NVIDIA
		std::ostream& operator<<(
			std::ostream& os,
			const vk::PhysicalDeviceShaderSMBuiltinsPropertiesNV& prop )
		{
			return os
				<< "\n\tShader SM Builtins Properties [NVIDIA]:"
				<< "\n\t\tShader SM Count: " << prop.shaderSMCount
				<< "\n\t\tShader Warps Per SM: " << prop.shaderWarpsPerSM;
		}

		std::ostream& operator<<(
			std::ostream& os,
			const vk::PhysicalDeviceShadingRateImagePropertiesNV& prop )
		{
			return os
				<< "\n\tShader Rate Image Properties [NVIDIA]:"
				<< "\n\t\tShading Rate Max Coarse Samples: " << prop.shadingRateMaxCoarseSamples
				<< "\n\t\tShading Rate Palette Size: " << prop.shadingRatePaletteSize
				<< "\n\t\tShading Rate Palette Size: " << "[x]=" << prop.shadingRateTexelSize.width
				<< " [y]=" << prop.shadingRateTexelSize.height;
		}


		//EXT
		std::ostream& operator<<(
			std::ostream& os,
			const vk::PhysicalDevicePCIBusInfoPropertiesEXT& prop )
		{
			return os
				<< "\n\tPCI Bus Info Properties:"
				<< "\n\t\tPCI Domain: " << prop.pciDomain
				<< "\n\t\tPCI Bus: " << prop.pciBus
				<< "\n\t\tPCI Device: " << prop.pciDevice
				<< "\n\t\tPCI Function: " << prop.pciFunction;
		}



		bool has_property(
			const std::vector<vk::ExtensionProperties>& properties,
			const std::string& property_name )
		{
			const auto&& iter {
				std::find_if(
					properties.begin(),
					properties.end(),
					[&property_name]( vk::ExtensionProperties const& ep )
					{
						return property_name == ep.extensionName;
					}
				)
			};
			return iter != properties.end();
		}

		template <typename T_property>
		void print_property( const auto& physical_device_properties )
		{
			using namespace fgl::vulkan::internal::properties_output;
			std::cout << ( physical_device_properties.template get<T_property>() ) << '\n';
		}
	} // namespace internal::properties_output

	void Context::print_debug_info() const
	{
		const auto& [loaded_version, target_version] { version_info };
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

		using namespace internal::properties_output;

		std::vector<vk::ExtensionProperties> extensionProperties {
			physical_device.enumerateDeviceExtensionProperties()
		};

		const auto& physical_device_properties {
			physical_device.getProperties2
			<
				vk::PhysicalDeviceProperties2,
				vk::PhysicalDeviceShaderCorePropertiesAMD,
				vk::PhysicalDeviceShaderCoreProperties2AMD,
				vk::PhysicalDeviceShaderSMBuiltinsPropertiesNV,
				vk::PhysicalDeviceShadingRateImagePropertiesNV,
				vk::PhysicalDevicePCIBusInfoPropertiesEXT
			>()
		};

		if( has_property( extensionProperties, "VK_AMD_shader_core_properties2" ) )
		{
			print_property<vk::PhysicalDeviceShaderCorePropertiesAMD>( physical_device_properties );
		}

		if( has_property( extensionProperties, "VK_AMD_shader_core_properties2" ) )
		{
			print_property<vk::PhysicalDeviceShaderCoreProperties2AMD>( physical_device_properties );
		}

		if( has_property( extensionProperties, "VK_NV_shader_sm_builtins" ) )
		{
			print_property<vk::PhysicalDeviceShaderSMBuiltinsPropertiesNV>( physical_device_properties );
		}

		if( has_property( extensionProperties, "VK_NV_shading_rate_image" ) )
		{
			print_property<vk::PhysicalDeviceShadingRateImagePropertiesNV>( physical_device_properties );
		}

		if( has_property( extensionProperties, "VK_EXT_pci_bus_info" ) )
		{
			print_property<vk::PhysicalDevicePCIBusInfoPropertiesEXT>( physical_device_properties );
		}

		std::cout << "\n\tMax Compute Work Group Sizes: ";

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
			<< std::endl;
	}
}
