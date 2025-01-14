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

		wgpu::InstanceDescriptor desc = {};
		desc.setDefault();
		wgpu::Instance instance = wgpu::createInstance(desc);
		if (!instance) {
			std::cerr << "Could not initialize WebGPU!\n";
			return false;
		}

		//TODO: Create or download logging library!
		std::cout << "WGPU instance: " << instance << "\n";

		//Get the surface to connect glfw with webgpu
		wgpu::Surface wgpuSurface = m_Window->GetGraphicsContext(instance);

		//Get the adapter
		wgpu::RequestAdapterOptions requestAdapterOpt = {};
		requestAdapterOpt.powerPreference = WGPUPowerPreference_HighPerformance;
		requestAdapterOpt.compatibleSurface = wgpuSurface;
		requestAdapterOpt.setDefault();

		wgpu::Adapter adapter = instance.requestAdapter(requestAdapterOpt);

		instance.release();

		//Get adapter features
		size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);
		m_Features.resize(featureCount);
		wgpuAdapterEnumerateFeatures(adapter, m_Features.data());

		//Get adapter properties, usefull info for user of the program
		m_Properties.setDefault();
		adapter.getProperties(&m_Properties);

		//Create webgpu 'device' + 'device specifics'
		m_Device = new GraphicsDevice();
		m_Device->Initialize(adapter, m_Window->GetNativeWindow());

		m_Queue = m_Device->GetDeviceQueue();
		auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void* /* pUserData */) {
			std::cout << "Queued work finished with status: " << status << "\n";
		};
		wgpuQueueOnSubmittedWorkDone(m_Queue, onQueueWorkDone, nullptr /* pUserData */);


		m_Renderer = new Renderer();
		m_Renderer->Initialize(wgpuSurface, adapter);
		adapter.release();


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

		m_Queue.release();
		m_Device->Dispose();

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
		ImGui_ImplWGPU_Init(m_Device->GetNativeDevice(), 3, m_Renderer->GetSurfaceFormat());
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