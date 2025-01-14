#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu/webgpu.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"

#include "application.h"
#include "shader.h"
#include "utility.h"
#include "buffer.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

namespace dtr {

	Application* Application::m_Instance = nullptr;

	bool Application::initializeApplication()
	{
		m_Window = new Window("Learn WebGPU", WINDOW_WIDTH, WINDOW_HEIGHT);

		m_Context = new GraphicsContext();
		m_Context->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

		m_Renderer = new Renderer();
		m_Renderer->Initialize();

		if (!initializeImGui()) return false;
		
		//Initialize created application
		Initialize();

		return true;
	}

	void Application::terminateApplication()
	{
		Terminate(); //Terminate Application

		terminateImGui();

		m_Renderer->Release();

		m_Window->Terminate();
	}

	void Application::Run()
	{
		initializeApplication();

		m_IsRunning = true;

		while (m_IsRunning)
		{
			updateApplication();
			drawApplication();
		}

		terminateApplication();
	}

	void Application::updateApplication()
	{
		glfwPollEvents();

		//Update user application
		OnUpdate();

		OnUpdate();
	}

	void Application::drawApplication()
	{
		wgpu::RenderPassEncoder renderPass = m_Renderer->RendererBegin();
	
		OnDraw(renderPass);

		updateImGui(renderPass);

		m_Renderer->RendererEnd(renderPass);
	}

	bool Application::initializeImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOther(m_Window->GetNativeWindow(), true);
		ImGui_ImplWGPU_Init(m_Context->GetNativeDevice(), 3, m_Context->GetSurfaceFormat());
		return true;
	}

	void Application::terminateImGui()
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplWGPU_Shutdown();
	}

	void Application::updateImGui(wgpu::RenderPassEncoder renderPass)
	{
		ImGui_ImplWGPU_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// [...] Build our UI
		OnGuiDraw();

		// Draw the UI
		ImGui::EndFrame();
		// Convert the UI defined above into low-level drawing commands
		ImGui::Render();
		// Execute the low-level drawing commands on the WebGPU backend
		ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
	}

}