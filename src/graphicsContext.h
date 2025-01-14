#pragma once
#include <webgpu/webgpu.hpp>

#include "graphicsDevice.h"

namespace dtr
{

	class GraphicsContext
	{

	private:
		wgpu::Surface m_Surface;
		wgpu::TextureFormat m_SurfaceFormat = wgpu::TextureFormat::Undefined;

		GraphicsDevice* m_Device = nullptr;

	public:
		void Initialize(uint32_t width, uint32_t height);
		void Release();

		void SubmitToQueue(wgpu::CommandBuffer* cmd, size_t cmdCount);
		void Poll();
		void Swap();

		wgpu::Buffer CreateBuffer(wgpu::BufferDescriptor bufferDesc);
		void WriteBuffer(wgpu::Buffer buffer, void* data, size_t dataSize);

		wgpu::Device GetNativeDevice() { return m_Device->GetNativeDevice(); };
		wgpu::Surface GetNativeSurface() { return m_Surface; };
		wgpu::TextureFormat GetSurfaceFormat() { return m_SurfaceFormat; };

	private:

	};

}