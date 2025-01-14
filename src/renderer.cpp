#include "renderer.h"

#include "application.h"
#include "shader.h"

namespace dtr
{


	void Renderer::Initialize(wgpu::Surface surface, wgpu::Adapter adapter)
	{
		m_Surface = surface; //TODO: Maybe call refrence? To inc ref counter

		m_SurfaceFormat = wgpuSurfaceGetPreferredFormat(m_Surface, adapter);
		if (m_SurfaceFormat == WGPUTextureFormat_Undefined) {
			std::cerr << "Invalid surface format!" << "\n";
			return;
		}

		wgpu::SurfaceConfiguration config = {};
		config.setDefault();
		config.width = 1280; //TODO: Maybe make some sort of graphicsContext object that stores everything regarding the drawing surface and window information
		config.height = 720;
		config.format = m_SurfaceFormat;
		config.usage = WGPUTextureUsage_RenderAttachment;
		config.device = Application::Get()->m_Device->GetNativeDevice();
		config.presentMode = WGPUPresentMode_Fifo;
		config.alphaMode = WGPUCompositeAlphaMode_Auto;

		// Ensure surface configuration is applied before acquiring texture
		m_Surface.configure(config);

		//RENDERPIPELINE
		Shader shader("assets/shaders/triangle.wgsl");
		wgpu::ShaderModule shaderModule = shader.GetShaderModule(Application::Get()->m_Device->GetNativeDevice());

		//TODO: Vertex Attributes abstraheren
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
		blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
		blendState.alpha.dstFactor = wgpu::BlendFactor::One;
		blendState.alpha.operation = wgpu::BlendOperation::Add;

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

		{
			initializeUniforms();

			//Create pipelineLayout using the BindGroupLayouts.
			wgpu::PipelineLayoutDescriptor pipelineLayoutDesc{};
			pipelineLayoutDesc.bindGroupLayoutCount = 1;
			pipelineLayoutDesc.bindGroupLayouts = m_UniformBuffer->GetLayout();

			m_PipelineLayout = Application::Get()->m_Device->GetNativeDevice().createPipelineLayout(pipelineLayoutDesc);
		}

		pipelineDesc.layout = m_PipelineLayout;

		m_RenderPipeline = Application::Get()->m_Device->GetNativeDevice().createRenderPipeline(pipelineDesc);
	}

	wgpu::RenderPassEncoder Renderer::RendererBegin()
	{
		m_TargetView = getNextTextureView();
		if (!m_TargetView) return nullptr;

		//Create and Submit commands/Render queue
		wgpu::CommandEncoderDescriptor encoderDesc = {};
		encoderDesc.setDefault();
		encoderDesc.label = "My command encoder";
		m_CommandEncoder = Application::Get()->m_Device->GetNativeDevice().createCommandEncoder(encoderDesc);
			
		wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
		renderPassColorAttachment.setDefault();
		renderPassColorAttachment.view = m_TargetView;
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
		wgpu::RenderPassEncoder renderPass = m_CommandEncoder.beginRenderPass(renderPassDesc);

		//Do something with the renderpass
		renderPass.setPipeline(m_RenderPipeline);
	
		m_UniformBuffer->Bind(renderPass);

		{ //TODO: Make the setting of data easier to use
			float time = static_cast<float>(glfwGetTime());
			UniformData uniforms;
			uniforms.time = time;
			uniforms.color = { 0.0f, 1.0f, 0.0f, 1.0f };

			m_UniformBuffer->UpdateData(&uniforms, sizeof(UniformData));
		}
		return renderPass;
	}

	void Renderer::RendererEnd(wgpu::RenderPassEncoder renderPass)
	{
		renderPass.end();
		renderPass.release();

		wgpu::CommandBufferDescriptor cmdBufferDescriptor = {};
		cmdBufferDescriptor.setDefault();
		cmdBufferDescriptor.label = "Command buffer";
		wgpu::CommandBuffer command = m_CommandEncoder.finish(cmdBufferDescriptor);

		m_CommandEncoder.release();
		Application::Get()->m_Device->GetDeviceQueue().submit(1, &command);

		Application::Get()->m_Device->GetNativeDevice().poll(false);

		//Release target view and present surface after
		m_TargetView.release();
		m_Surface.present();
	}

	void Renderer::Release()
	{
		{ //TODO: Eventually up to user?
			m_UniformBuffer->Release();
		}

		m_RenderPipeline.release();
		m_PipelineLayout.release();

		m_Surface.unconfigure();
		m_Surface.release();
	}

	wgpu::TextureFormat Renderer::GetSurfaceFormat()
	{
		return m_SurfaceFormat;
	}

	void Renderer::initializeUniforms()
	{
		m_UniformBuffer = new UniformBuffer();

		UniformData uniforms;
		uniforms.time = 1.0f;
		uniforms.color = { 1.0f, 1.0f, 1.0f, 1.0f };

		m_UniformBuffer->SetData(&uniforms, sizeof(UniformData));
	}

	wgpu::TextureView Renderer::getNextTextureView()
	{
		wgpu::SurfaceTexture surfaceTexture;
		m_Surface.getCurrentTexture(&surfaceTexture);
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