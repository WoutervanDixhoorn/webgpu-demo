#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu/webgpu.hpp"
#include <glfw3webgpu.h>

#include "application.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

namespace dtr {

	bool Application::Initialize()
	{
		if (!glfwInit()) {
			std::cerr << "Could not initialize GLFW!" << std::endl;
			return 1;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // <-- extra info for glfwCreateWindow
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_Window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Learn WebGPU", nullptr, nullptr);
		if (!m_Window) {
			std::cerr << "Could not open window!" << std::endl;
			glfwTerminate();
			return 1;
		}

		m_WindowData = new WindowData();
		m_WindowData->m_Application = this;
		glfwSetWindowUserPointer(m_Window, (void*)m_WindowData);

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			WindowData* windowData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			if (windowData)
				windowData->m_Application->stopRunning();
		});

		wgpu::InstanceDescriptor desc = {};
		desc.setDefault();
		wgpu::Instance instance = wgpu::createInstance(desc);
		if (!instance) {
			std::cerr << "Could not initialize WebGPU!\n";
			return false;
		}

		//TODO: Create or download logging library!
		std::cout << "WGPU instance: " << instance << "\n";

		//Get the surface to connect glfw with webgpu
		m_WGPUSurface = glfwGetWGPUSurface(instance, m_Window);

		//Get the adapter
		wgpu::RequestAdapterOptions requestAdapterOpt = {};
		requestAdapterOpt.powerPreference = WGPUPowerPreference_LowPower;
		requestAdapterOpt.compatibleSurface = m_WGPUSurface;
		requestAdapterOpt.setDefault();

		wgpu::Adapter adapter = instance.requestAdapter(requestAdapterOpt);

		instance.release();

		//Also using old api, wbgpu.hpp wont work
		//Get adapter features
		size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);
		m_Features.resize(featureCount);
		wgpuAdapterEnumerateFeatures(adapter, m_Features.data());

		//Get adapter properties, usefull info for user of the program
		m_Properties.setDefault();
		adapter.getProperties(&m_Properties);

		//Create webgpu 'device' + 'device specifics'
		m_Device = new GraphicsDevice();
		m_Device->Initialize(adapter, m_Window);

		m_Queue = m_Device->GetDeviceQueue();
		auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void* /* pUserData */) {
			std::cout << "Queued work finished with status: " << status << std::endl;
		};
		wgpuQueueOnSubmittedWorkDone(m_Queue, onQueueWorkDone, nullptr /* pUserData */);

		wgpu::TextureFormat surfaceFormat = wgpuSurfaceGetPreferredFormat(m_WGPUSurface, adapter);
		if (surfaceFormat == WGPUTextureFormat_Undefined) {
			std::cerr << "Invalid surface format!" << std::endl;
			return false;
		}

		wgpu::SurfaceConfiguration config = {};
		config.setDefault();
		config.width = WINDOW_WIDTH;
		config.height = WINDOW_HEIGHT;
		config.format = surfaceFormat;
		config.usage = WGPUTextureUsage_RenderAttachment;
		config.device = m_Device->GetNativeDevice();
		config.presentMode = WGPUPresentMode_Fifo;
		config.alphaMode = WGPUCompositeAlphaMode_Auto;

		// Ensure surface configuration is applied before acquiring texture
		m_WGPUSurface.configure(config);

		adapter.release();

		return true;
	}

	void Application::Terminate()
	{
		m_WGPUSurface.unconfigure();
		m_WGPUSurface.release();

		m_Queue.release();
		m_Device->Dispose();

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void Application::Update()
	{
		glfwPollEvents();

		//Get target view
		wgpu::TextureView targetView = GetNextSurfaceTextureView();
		if (!targetView) return;

		//Create and Submit commands/Render queue
		wgpu::CommandEncoderDescriptor encoderDesc = {};
		encoderDesc.setDefault();
		encoderDesc.label = "My command encoder";
		wgpu::CommandEncoder encoder = m_Device->GetNativeDevice().createCommandEncoder(encoderDesc);

		wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
		renderPassColorAttachment.setDefault();
		renderPassColorAttachment.view = targetView;
		renderPassColorAttachment.resolveTarget = nullptr;
		renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
		renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
		renderPassColorAttachment.clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 };

		wgpu::RenderPassDescriptor renderPassDesc = {};
		renderPassDesc.setDefault();
		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;
		renderPassDesc.depthStencilAttachment = nullptr;
		renderPassDesc.timestampWrites = nullptr;
		wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

		//Do something with the renderpass

		renderPass.end();
		renderPass.release();

		wgpu::CommandBufferDescriptor cmdBufferDescriptor = {};
		cmdBufferDescriptor.setDefault();
		cmdBufferDescriptor.label = "Command buffer";
		wgpu::CommandBuffer command = encoder.finish(cmdBufferDescriptor);
		encoder.release();

		m_Queue.submit(1, &command);
		command.release();

		wgpuDevicePoll(m_Device->GetNativeDevice(), false, nullptr);

		//Release target view and present surface after
		targetView.release();
		m_WGPUSurface.present();
	}

	wgpu::TextureView Application::GetNextSurfaceTextureView()
	{
		wgpu::SurfaceTexture surfaceTexture;
		m_WGPUSurface.getCurrentTexture(&surfaceTexture);
		wgpu::Texture texture = surfaceTexture.texture;
		if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
			std::cerr << "Failed getting current texture. status: " << surfaceTexture.status << "\n";
			return nullptr;
		}

		wgpu::TextureViewDescriptor viewDescriptor;
		viewDescriptor.setDefault();
		viewDescriptor.label = "Surface texture view";
		viewDescriptor.format = texture.getFormat();
		viewDescriptor.dimension = WGPUTextureViewDimension_2D;
		viewDescriptor.baseMipLevel = 0;
		viewDescriptor.mipLevelCount = 1;
		viewDescriptor.baseArrayLayer = 0;
		viewDescriptor.arrayLayerCount = 1;
		viewDescriptor.aspect = wgpu::TextureAspect::All;

		WGPUTextureView targetView = texture.createView(viewDescriptor);

		return targetView;
	}

}