#include "buffer.h"

#include "application.h"

namespace dtr
{
	//--------------------------------------------------------------------------------
	//					VERTEX BUFFER
	//--------------------------------------------------------------------------------

	void VertexBuffer::Bind(wgpu::RenderPassEncoder renderPass)
	{
		renderPass.setVertexBuffer(0, m_Buffer, 0, m_Buffer.getSize());
	}

	//TODO: Currently only support float vertices, make more types available
	void VertexBuffer::SetData(float* data, uint64_t count)
	{
		m_Data = data;
		m_VertexCount = count;

		wgpu::BufferDescriptor bufferDesc;
		bufferDesc.size = m_VertexCount * sizeof(float);
		bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex; // Vertex usage here!
		bufferDesc.mappedAtCreation = false;
		m_Buffer = Application::Get()->m_Device->GetNativeDevice().createBuffer(bufferDesc);

		Application::Get()->m_Device->GetDeviceQueue().writeBuffer(m_Buffer, 0, m_Data, bufferDesc.size);
	}

	void VertexBuffer::Release()
	{
		m_Buffer.release();
	}

	//--------------------------------------------------------------------------------
	//					INDEX BUFFER
	//--------------------------------------------------------------------------------

	void IndexBuffer::Bind(wgpu::RenderPassEncoder renderPass)
	{
		renderPass.setIndexBuffer(m_Buffer, wgpu::IndexFormat::Uint16, 0, m_Buffer.getSize());
	}

	void IndexBuffer::SetData(uint16_t* data, uint64_t count)
	{
		m_Data = data;
		m_IndexCount = count;

		wgpu::BufferDescriptor bufferDesc;
		bufferDesc.size = count * sizeof(uint16_t); //IndexBuffer uses uint16 as standard type
		bufferDesc.size = (bufferDesc.size + 3) & ~3;
		bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
		bufferDesc.mappedAtCreation = false;
		m_Buffer = Application::Get()->m_Device->GetNativeDevice().createBuffer(bufferDesc);

		Application::Get()->m_Device->GetDeviceQueue().writeBuffer(m_Buffer, 0, m_Data, bufferDesc.size);
	}

	void IndexBuffer::Release()
	{
		m_Buffer.release();
	}
}