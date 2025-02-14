#pragma once
#include <webgpu/webgpu.hpp>

#include <array>

#include "graphicsContext.h"
#include "buffer.h"

namespace dtr
{
	struct UniformData
	{
		std::array<float, 4> color; // 4 * sizeod(float) = 16 bytes

		float time;					// 1 * sizeof(float) = 4 bytes
		float _pad[3]; //Padding    // 3 * sizeof(float) = 12 bytes
		// -----------------------+	
		//						32 bytes to fill memory correctly 
	};

	class Renderer {
	private:
		GraphicsContext* m_Context;

		wgpu::RenderPipeline m_RenderPipeline;
		wgpu::PipelineLayout m_PipelineLayout;

		UniformBuffer* m_UniformBuffer;

		wgpu::TextureView m_TargetView;
		wgpu::CommandEncoder m_CommandEncoder;

	public:
		void Initialize();
		void Release();


		wgpu::RenderPassEncoder RendererBegin();
		void RendererEnd(wgpu::RenderPassEncoder renderPass);

		wgpu::TextureFormat GetSurfaceFormat(); //TODO: Add this to some sort of RenderContext object

	private:
		void initializeUniforms();

		wgpu::TextureView getNextTextureView();
		
		
	};

}