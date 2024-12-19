#pragma once

#include <vector>

#include <webgpu/webgpu.hpp>
#include <GLFW/glfw3.h>

namespace dtr {

	class GraphicsDevice {
	private:
		wgpu::Queue m_Queue;
		wgpu::Device m_Device;

	public:
		bool Initialize(wgpu::Adapter adapter, GLFWwindow* window);
		bool Dispose();
		void InspectDevice();

		inline wgpu::Device GetNativeDevice() { return m_Device; }
		inline wgpu::Queue GetDeviceQueue() { return m_Device.getQueue(); }

		void Poll() { m_Device.poll(false); }

	private:
		wgpu::RequiredLimits GetRequiredLimits(wgpu::Adapter adapter) const;
	};

}