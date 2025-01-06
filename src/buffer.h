#pragma once
#include <webgpu/webgpu.hpp>
#include <vector>

namespace dtr
{

	class VertexBuffer
	{
	private:
		wgpu::Buffer m_Buffer;

		float* m_Data;
		uint64_t m_VertexCount;
		
	public:
		void Bind(wgpu::RenderPassEncoder renderPass); //Todo: replace this so that no renderPass needs to be passed into the object.

		void SetData(float* data, uint64_t count);

		void Release();
		

	};

	class IndexBuffer
	{
	private:
		wgpu::Buffer m_Buffer;

		uint16_t* m_Data;
		uint64_t m_IndexCount;
		size_t m_VertexSize;

	public:
		void Bind(wgpu::RenderPassEncoder renderPass); //Todo: replace this so that no renderPass needs to be passed into the object.

		void SetData(uint16_t* data, uint64_t count);

		void Release();

		inline uint64_t GetCount() { return m_IndexCount; }
	};
}