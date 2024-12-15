

#include "graphicsDevice.h"
#include "shader.h"

namespace dtr {

	bool GraphicsDevice::Initialize(wgpu::Adapter adapter, GLFWwindow* window)
	{	
		wgpu::DeviceDescriptor deviceDesc = {};
		deviceDesc.setDefault();
		deviceDesc.label = "DouterGPU";
		deviceDesc.requiredFeatureCount = 0;
		deviceDesc.requiredLimits = nullptr;
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

		//if (!InitializeRenderPipeline(adapter)) {
		//	std::cout << "Failed initializing render pipeline" << std::endl;
		//}

		InspectDevice();

		return true;
	}

	//bool GraphicsDevice::InitializeRenderPipeline(wgpu::Adapter adapter)
	//{
	//	Shader shader("triangle.wgsl");

	//	wgpu::ShaderModuleDescriptor shaderDesc;
	//	shaderDesc.hintCount = 0;
	//	shaderDesc.hints = nullptr;

	//	wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
	//	shaderCodeDesc.chain.next = nullptr;
	//	shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
	//	shaderCodeDesc.code = shader.GetShaderSource();

	//	shaderDesc.nextInChain = &shaderCodeDesc.chain;

	//	wgpu::ShaderModule shaderModule = m_Device.createShaderModule(shaderDesc);

	//	wgpu::RenderPipelineDescriptor pipelineDesc;
	//	pipelineDesc.setDefault();

	//	pipelineDesc.vertex.bufferCount = 0;
	//	pipelineDesc.vertex.buffers = nullptr;
	//	pipelineDesc.vertex.module = shaderModule;
	//	pipelineDesc.vertex.entryPoint = "vs_main";
	//	pipelineDesc.vertex.constantCount = 0;
	//	pipelineDesc.vertex.constants = nullptr;

	//	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	//	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
	//	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
	//	pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

	//	wgpu::BlendState blendState;
	//	blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
	//	blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	//	blendState.color.operation = wgpu::BlendOperation::Add;
	//	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	//	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	//	blendState.alpha.operation = wgpu::BlendOperation::Add;

	//	wgpu::ColorTargetState colorTarget;
	//	colorTarget.format = m_WGPUSurface.getPreferredFormat(adapter);
	//	colorTarget.blend = &blendState;
	//	colorTarget.writeMask = wgpu::ColorWriteMask::All;

	//	wgpu::FragmentState fragmentState;
	//	fragmentState.module = shaderModule;
	//	fragmentState.entryPoint = "fs_main";
	//	fragmentState.constantCount = 0;
	//	fragmentState.constants = nullptr;
	//	fragmentState.targetCount = 1;
	//	fragmentState.targets = &colorTarget;

	//	pipelineDesc.fragment = &fragmentState;
	//	pipelineDesc.depthStencil = nullptr;

	//	pipelineDesc.multisample.count = 1;
	//	pipelineDesc.multisample.mask = ~0u;
	//	pipelineDesc.multisample.alphaToCoverageEnabled = false;

	//	pipelineDesc.layout = nullptr;

	//	m_RenderPipeline = m_Device.createRenderPipeline(pipelineDesc);

	//	shaderModule.release();

	//	return true;
	//}

	bool GraphicsDevice::Dispose()
	{
		//m_RenderPipeline.release();
		m_Device.release();
		return true;
	}

	//wgpu::TextureView GraphicsDevice::GetNextSurfaceTextureView()
	//{
	//	wgpu::SurfaceTexture surfaceTexture;
	//	m_WGPUSurface.getCurrentTexture(&surfaceTexture);
	//	wgpu::Texture texture = surfaceTexture.texture;
	//	if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
	//		return nullptr;
	//	}

	//	wgpu::TextureViewDescriptor viewDescriptor;
	//	viewDescriptor.setDefault();
	//	viewDescriptor.label = "Surface texture view";
	//	viewDescriptor.format = texture.getFormat();
	//	viewDescriptor.dimension = WGPUTextureViewDimension_2D;
	//	viewDescriptor.baseMipLevel = 0;
	//	viewDescriptor.mipLevelCount = 1;
	//	viewDescriptor.baseArrayLayer = 0;
	//	viewDescriptor.arrayLayerCount = 1;
	//	viewDescriptor.aspect = wgpu::TextureAspect::All;

	//	WGPUTextureView targetView = texture.createView(viewDescriptor);

	//	return targetView;
	//}

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
}