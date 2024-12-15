#include <iostream>
#include <vector>

#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>

#include "application.h"

int main(int, char**) {
	dtr::Application* app = new dtr::Application();

	if (!app->Initialize()){
		return 1;
	};

	while (app->IsRunning())
	{
		app->Update();
	}

	app->Terminate();

	return 0;
}