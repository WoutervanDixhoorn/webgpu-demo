#pragma once
#include <GLFW/glfw3.h>
#include "glfw3webgpu.h"

#include <webgpu/webgpu.hpp>

namespace dtr
{
	class Window
	{
		struct WindowData {
			int m_Width, m_Height;
			const char* m_Title;
		};

	private:
		WindowData* m_WindowData = nullptr;
		GLFWwindow* m_Window;

	public:
		Window(const char* title, int width, int height);
		~Window();
		
		void Terminate();

		inline GLFWwindow* GetNativeWindow() { return m_Window; };
		wgpu::Surface GetGraphicsContext(wgpu::Instance wgpuInstance) { return glfwGetWGPUSurface(wgpuInstance, m_Window); };
		
	private:
		bool initialize();

	};

}