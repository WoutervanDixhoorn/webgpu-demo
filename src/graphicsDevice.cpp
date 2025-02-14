#include "graphicsDevice.h"
#include "shader.h"

namespace dtr {

	bool GraphicsDevice::Initialize(wgpu::Adapter adapter)
	{	
		wgpu::RequiredLimits requiredLimits = GetRequiredLimits(adapter);

		wgpu::DeviceDescriptor deviceDesc = {};
		deviceDesc.setDefault();
		deviceDesc.label = "DouterGPU";
		deviceDesc.requiredFeatureCount = 0;
		deviceDesc.requiredLimits = &requiredLimits;
		deviceDesc.defaultQueue.nextInChain = nullptr;
		deviceDesc.defaultQueue.label = "The default queue";
		deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, const char* message, void* /* pUserData */) {
			std::cout << "Device lost: reason " << reason;
			if (message) std::cout << " (" << message << ")";
			std::cout << std::endl;
		};

		std::cout << "Requesting device..." << std::endl;
		m_Device = adapter.requestDevice(deviceDesc);
		std::cout << "Got device: " << m_Device << std::endl;

		auto uncapturedErrorCallback = [](wgpu::ErrorType type, char const* message) {
			std::cout << "Uncaptured device error: type " << type;
			if (message) std::cout << " (" << message << ")";
			std::cout << std::endl;
		};
		m_Device.setUncapturedErrorCallback(uncapturedErrorCallback);

		auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void* /* pUserData */) {
			std::cout << "Queued work finished with status: " << status << "\n";
			};
		wgpuQueueOnSubmittedWorkDone(GetDeviceQueue(), onQueueWorkDone, nullptr /* pUserData */);

		InspectDevice();

		return true;
	}

	void GraphicsDevice::Release()
	{
		m_Device.release();
	}

	void GraphicsDevice::InspectDevice()
	{
		std::vector<WGPUFeatureName> features;
		size_t featureCount = wgpuDeviceEnumerateFeatures(m_Device, nullptr);
		features.resize(featureCount);
		wgpuDeviceEnumerateFeatures(m_Device, features.data());

		std::cout << "Device features:" << std::endl;
		std::cout << std::hex;
		for (auto f : features) {
			std::cout << " - 0x" << f << std::endl;
		}
		std::cout << std::dec;

		WGPUSupportedLimits limits = {};
		limits.nextInChain = nullptr;

		bool success = wgpuDeviceGetLimits(m_Device, &limits);

		if (success) {
			std::cout << "Device limits:" << std::endl;
			std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << std::endl;
			std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << std::endl;
			std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << std::endl;
			std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << std::endl;
		}
	}

	wgpu::RequiredLimits GraphicsDevice::GetRequiredLimits(wgpu::Adapter adapter) const
	{
		wgpu::SupportedLimits supportedLimits;
		adapter.getLimits(&supportedLimits);

		wgpu::RequiredLimits requiredLimits = wgpu::Default;

		//Only the minimum required limits to run this application
		requiredLimits.limits.maxVertexAttributes = 3;
		requiredLimits.limits.maxVertexBuffers = 1;
		requiredLimits.limits.maxBufferSize = 150000 * (5 * sizeof(float));
		requiredLimits.limits.maxVertexBufferArrayStride = 5 * sizeof(float);
		// There is a maximum of 3 float forwarded from vertex to fragment shader
		requiredLimits.limits.maxInterStageShaderComponents = 8;

		requiredLimits.limits.maxBindGroups = 2;
		requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
		requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float); //Means the bufferbinding can't be bigger then 16 times 4 floats 

		requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
		requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;

		return requiredLimits;
	}
}