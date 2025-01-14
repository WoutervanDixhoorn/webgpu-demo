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

	//--------------------------------------------------------------------------------
	//					UNIFORM BUFFER
	//--------------------------------------------------------------------------------

	void UniformBuffer::Bind(wgpu::RenderPassEncoder renderPass)
	{
		renderPass.setBindGroup(0, m_BindGroup, 0, nullptr);
	}

	void UniformBuffer::SetData(void* uniformData, uint64_t uniformSize)
	{
		//Initialize the buffer
		wgpu::BufferDescriptor bufferDesc;
		bufferDesc.size = uniformSize;
		bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
		bufferDesc.mappedAtCreation = false;
		m_Buffer = Application::Get()->m_Device->GetNativeDevice().createBuffer(bufferDesc);

		Application::Get()->m_Device->GetDeviceQueue().writeBuffer(m_Buffer, 0, &uniformData, uniformSize);
		

		//Initialize layout
		wgpu::BindGroupLayoutEntry bindGroupLayoutEntry = wgpu::Default;
		bindGroupLayoutEntry.binding = 0; // @binding(0) attribute in shader //TODO: At this point int time we just support 1 uniform binding
		bindGroupLayoutEntry.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
		bindGroupLayoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
		bindGroupLayoutEntry.buffer.minBindingSize = uniformSize;

		wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
		bindGroupLayoutDesc.entryCount = 1;
		bindGroupLayoutDesc.entries = &bindGroupLayoutEntry;
		m_Layout = Application::Get()->m_Device->GetNativeDevice().createBindGroupLayout(bindGroupLayoutDesc);
		
		
		{ //Initialize bindGroup
			wgpu::BindGroupEntry binding{};
			binding.binding = 0;
			binding.buffer = m_Buffer; //wgpuBuffer object
			binding.offset = 0;
			binding.size = uniformSize; //Size of the data

			wgpu::BindGroupDescriptor bindGroupDesc{};
			bindGroupDesc.layout = m_Layout;
			bindGroupDesc.entryCount = 1;
			bindGroupDesc.entries = &binding;
			m_BindGroup = Application::Get()->m_Device->GetNativeDevice().createBindGroup(bindGroupDesc);
		}

		m_UniformSize = uniformSize;
	}
	
	void UniformBuffer::UpdateData(void* uniformData, uint64_t dataSize)
	{
		Application::Get()->m_Device->GetDeviceQueue().writeBuffer(m_Buffer, 0, uniformData, dataSize);
	}

	WGPUBindGroupLayout* UniformBuffer::GetLayout()
	{
		return (WGPUBindGroupLayout*)&m_Layout;
	}

	void UniformBuffer::Release()
	{
		m_Buffer.release();
	}
}