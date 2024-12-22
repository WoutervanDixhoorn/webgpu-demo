#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu/webgpu.hpp"
#include <glfw3webgpu.h>

#include "application.h"
#include "shader.h"
#include "utility.h"

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
		requestAdapterOpt.powerPreference = WGPUPowerPreference_HighPerformance;
		requestAdapterOpt.compatibleSurface = m_WGPUSurface;
		requestAdapterOpt.setDefault();

		wgpu::Adapter adapter = instance.requestAdapter(requestAdapterOpt);

		instance.release();

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
			std::cout << "Queued work finished with status: " << status << "\n";
		};
		wgpuQueueOnSubmittedWorkDone(m_Queue, onQueueWorkDone, nullptr /* pUserData */);

		m_SurfaceFormat = wgpuSurfaceGetPreferredFormat(m_WGPUSurface, adapter);
		if (m_SurfaceFormat == WGPUTextureFormat_Undefined) {
			std::cerr << "Invalid surface format!" << "\n";
			return false;
		}

		wgpu::SurfaceConfiguration config = {};
		config.setDefault();
		config.width = WINDOW_WIDTH;
		config.height = WINDOW_HEIGHT;
		config.format = m_SurfaceFormat;
		config.usage = WGPUTextureUsage_RenderAttachment;
		config.device = m_Device->GetNativeDevice();
		config.presentMode = WGPUPresentMode_Fifo;
		config.alphaMode = WGPUCompositeAlphaMode_Auto;

		// Ensure surface configuration is applied before acquiring texture
		m_WGPUSurface.configure(config);

		adapter.release();

		InitializeBuffers();
		InitializeRenderPipeline();

		return true;
	}

	void Application::InitializeRenderPipeline()
	{
		Shader shader("assets/shaders/triangle.wgsl");
		wgpu::ShaderModule shaderModule = shader.GetShaderModule(m_Device->GetNativeDevice());

		std::vector<wgpu::VertexAttribute> vertexAttribs(2);

		vertexAttribs[0].shaderLocation = 0;
		vertexAttribs[0].format = wgpu::VertexFormat::Float32x2;
		vertexAttribs[0].offset = 0;

		vertexAttribs[1].shaderLocation = 1;
		vertexAttribs[1].format = wgpu::VertexFormat::Float32x3;
		vertexAttribs[1].offset = 2 * sizeof(float);

		wgpu::VertexBufferLayout vertexBufferLayout;
		vertexBufferLayout.attributeCount = static_cast<uint32_t>(vertexAttribs.size());
		vertexBufferLayout.attributes = vertexAttribs.data();
		vertexBufferLayout.arrayStride = 5 * sizeof(float);
		vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

		wgpu::RenderPipelineDescriptor pipelineDesc = {};
		pipelineDesc.vertex.bufferCount = 1;
		pipelineDesc.vertex.buffers = &vertexBufferLayout;
		pipelineDesc.vertex.module = shaderModule;
		pipelineDesc.vertex.entryPoint = "vs_main";
		pipelineDesc.vertex.constantCount = 0;
		pipelineDesc.vertex.constants = nullptr;

		pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
		pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
		pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
		pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

		wgpu::FragmentState fragmentState;
		fragmentState.module = shaderModule;
		fragmentState.entryPoint = "fs_main";
		fragmentState.constantCount = 0;
		fragmentState.constants = nullptr;

		wgpu::BlendState blendState;
		blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
		blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
		blendState.color.operation = wgpu::BlendOperation::Add;

		wgpu::ColorTargetState colorTarget;
		colorTarget.format = m_SurfaceFormat;
		colorTarget.blend = &blendState;
		colorTarget.writeMask = wgpu::ColorWriteMask::All;

		fragmentState.targetCount = 1;
		fragmentState.targets = &colorTarget;
		pipelineDesc.fragment = &fragmentState;

		pipelineDesc.depthStencil = nullptr;

		pipelineDesc.multisample.count = 1;
		pipelineDesc.multisample.mask = ~0u;
		pipelineDesc.multisample.alphaToCoverageEnabled = false;

		pipelineDesc.layout = nullptr;

		m_Pipeline = m_Device->GetNativeDevice().createRenderPipeline(pipelineDesc);
	}

	void Application::InitializeBuffers()
	{
		std::vector<float> vertexData = {
			-0.5, -0.5,   1.0, 0.0, 0.0,
			+0.5, -0.5,   0.0, 1.0, 0.0,
			+0.5, +0.5,   0.0, 0.0, 1.0,
			-0.5, +0.5,   1.0, 1.0, 0.0
		};

		std::vector<uint16_t> indexData = {
			0, 1, 2, 
			0, 2, 3 
		};

		LoadGeometry("assets/webgpu.txt", vertexData, indexData);

		m_IndexCount = static_cast<uint32_t>(indexData.size());

		wgpu::BufferDescriptor bufferDesc;
		bufferDesc.size = vertexData.size() * sizeof(float);
		bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex; // Vertex usage here!
		bufferDesc.mappedAtCreation = false;
		m_VertexBuffer = m_Device->GetNativeDevice().createBuffer(bufferDesc);

		m_Queue.writeBuffer(m_VertexBuffer, 0, vertexData.data(), bufferDesc.size);

		bufferDesc.size = indexData.size() * sizeof(uint16_t);
		bufferDesc.size = (bufferDesc.size + 3) & ~3; // round up to the next multiple of 4
		bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index; // Vertex usage here!
		m_IndexBuffer = m_Device->GetNativeDevice().createBuffer(bufferDesc);

		m_Queue.writeBuffer(m_IndexBuffer, 0, indexData.data(), bufferDesc.size);
	}

	void Application::Terminate()
	{
		m_VertexBuffer.release();

		m_Pipeline.release();

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
		renderPassColorAttachment.clearValue = WGPUColor{ 0.05, 0.05, 0.05, 1.0 };

		wgpu::RenderPassDescriptor renderPassDesc = {};
		renderPassDesc.setDefault();
		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;
		renderPassDesc.depthStencilAttachment = nullptr;
		renderPassDesc.timestampWrites = nullptr;
		wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

		//Do something with the renderpass
		renderPass.setPipeline(m_Pipeline);
		// Draw 1 instance of a 3-vertices shape
		renderPass.setVertexBuffer(0, m_VertexBuffer, 0, m_VertexBuffer.getSize());
		renderPass.setIndexBuffer(m_IndexBuffer, wgpu::IndexFormat::Uint16, 0, m_IndexBuffer.getSize());
		// We use the `vertexCount` variable instead of hard-coding the vertex count
		renderPass.drawIndexed(m_IndexCount, 1, 0, 0, 0);

		renderPass.end();
		renderPass.release();

		wgpu::CommandBufferDescriptor cmdBufferDescriptor = {};
		cmdBufferDescriptor.setDefault();
		cmdBufferDescriptor.label = "Command buffer";
		wgpu::CommandBuffer command = encoder.finish(cmdBufferDescriptor);
		encoder.release();

		m_Queue.submit(1, &command);
		command.release();

		m_Device->GetNativeDevice().poll(false);

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