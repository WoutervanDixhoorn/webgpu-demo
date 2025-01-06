#include "window.h"

#include <iostream>

#include "application.h"

namespace dtr
{

	Window::Window(const char* title, int width, int height)
	{
		m_WindowData = new WindowData();
		m_WindowData->m_Title = title;
		m_WindowData->m_Width = width;
		m_WindowData->m_Height = height;

		if (!initialize()) {
			std::cerr << "Failed initialing aGLFW window!\n";
			Terminate();
		}
		else {
			std::cout << "Succesfully initialized a GLFW window!\n";
		}
	}

	Window::~Window()
	{
		Terminate();
	}

	void Window::Terminate()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();

		delete m_WindowData;
	}

	bool Window::initialize()
	{
		if (!glfwInit()) {
			std::cerr << "Could not initialize GLFW!" << std::endl;
			return false;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // <-- extra info for glfwCreateWindow
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_Window = glfwCreateWindow(m_WindowData->m_Width, m_WindowData->m_Height, m_WindowData->m_Title, nullptr, nullptr);
		if (!m_Window) {
			std::cerr << "Could not open window!" << std::endl;
			glfwTerminate();
			return false;
		}

		return true;
	}

	void Window::SetWindowCallbacks(Application* applicationPtr)
	{
		m_WindowData->m_Application = applicationPtr;
		glfwSetWindowUserPointer(m_Window, (void*)m_WindowData);

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			WindowData* windowData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			if (windowData)
				windowData->m_Application->stopRunning();
		});
	}
}