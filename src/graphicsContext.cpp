#include "graphicsContext.h"

#include "application.h"

namespace dtr
{

	void GraphicsContext::Initialize(uint32_t width, uint32_t height)
	{
		wgpu::InstanceDescriptor desc = {};
		desc.setDefault();
		wgpu::Instance instance = wgpu::createInstance(desc);
		if (!instance) {
			std::cerr << "Could not initialize WebGPU!\n";
			return;
		}

		//TODO: Create or download logging library!
		std::cout << "WGPU instance: " << instance << "\n";

		//Create adapter
		wgpu::RequestAdapterOptions requestAdapterOpt = {};
		requestAdapterOpt.powerPreference = WGPUPowerPreference_HighPerformance;
		requestAdapterOpt.compatibleSurface = m_Surface;
		requestAdapterOpt.setDefault();

		wgpu::Adapter adapter = instance.requestAdapter(requestAdapterOpt);
		instance.release();

		m_Device = new GraphicsDevice();
		m_Device->Initialize(adapter);

		m_Surface = Application::Get()->m_Window->GetGraphicsContext(instance);
		m_SurfaceFormat = wgpuSurfaceGetPreferredFormat(m_Surface, adapter);
		if (m_SurfaceFormat == WGPUTextureFormat_Undefined) {
			std::cerr << "Invalid surface format!" << "\n";
			return;
		}

		wgpu::SurfaceConfiguration config = {};
		config.setDefault();
		config.width = width; //TODO: Maybe make some sort of graphicsContext object that stores everything regarding the drawing surface and window information
		config.height = height;
		config.format = m_SurfaceFormat;
		config.usage = WGPUTextureUsage_RenderAttachment;
		config.device = Application::Get()->m_Context->GetNativeDevice();
		config.presentMode = WGPUPresentMode_Fifo;
		config.alphaMode = WGPUCompositeAlphaMode_Auto;

		// Ensure surface configuration is applied before acquiring texture
		m_Surface.configure(config);
	}

	void GraphicsContext::Release()
	{
		m_Surface.unconfigure();
		m_Surface.release();

		m_Device->Release();
	}

	void GraphicsContext::SubmitToQueue(wgpu::CommandBuffer* cmd, size_t cmdCount)
	{
		m_Device->GetDeviceQueue().submit(cmdCount, cmd);
	}

	void GraphicsContext::Poll()
	{
		m_Device->Poll();
	}
	
	void GraphicsContext::Swap()
	{
		m_Surface.present();
	}

	wgpu::Buffer GraphicsContext::CreateBuffer(wgpu::BufferDescriptor bufferDesc)
	{
		return GetNativeDevice().createBuffer(bufferDesc);
	}

	void GraphicsContext::WriteBuffer(wgpu::Buffer buffer, void* data, size_t dataSize)
	{
		m_Device->GetDeviceQueue().writeBuffer(buffer, 0, data, dataSize);
	}
}