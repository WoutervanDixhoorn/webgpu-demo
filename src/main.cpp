#include <iostream>
#include <vector>

#include <webgpu/webgpu.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "application.h"
#include "utility.h"

class SandboxApp : public dtr::Application
{
private:
	wgpu::Buffer vertexBuffer;
	uint32_t m_IndexCount;

public:
	bool Initialize() override
	{
		std::cout << "Initialize Application\n";

		std::vector<float> vertexData;
		std::vector<uint16_t> indexData;
		dtr::LoadGeometry("assets/webgpu.txt", vertexData, indexData);

		m_IndexCount = static_cast<uint32_t>(indexData.size());



		return true;
	}

	void Terminate() override
	{
		std::cout << "Terminate Application\n";
	}

	void OnGuiDraw() override
	{
		static float f = 0.0f;
		static int counter = 0;
		static bool show_demo_window = true;
		static bool show_another_window = false;
		static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		ImGui::Begin("Hello, world!");                                // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");                     // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);            // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);                  // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color);       // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                                  // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

	}

	void OnUpdate() override
	{
	}

	void OnDraw(wgpu::RenderPassEncoder renderPass) override
	{
	}

};

dtr::Application* dtr::Application::CreateApplication()
{
	return new SandboxApp();
}

int main(int, char**) {
	dtr::Application* app = SandboxApp::Get();

	app->Run();

	delete app;

	return 0;
}