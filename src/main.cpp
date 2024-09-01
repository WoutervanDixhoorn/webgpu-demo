#include <iostream>
#include <vector>

#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>

WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const* options) {

	struct UserData {
		WGPUAdapter adapter = nullptr;
		bool requestEnded = false;
	};
	UserData userData;

	auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* pUserData) {
		UserData& userData = *reinterpret_cast<UserData*>(pUserData);
		if (status == WGPURequestAdapterStatus_Success) {
			userData.adapter = adapter;
		}
		else {
			std::cout << "Could not get WebGPU adapter: " << message << std::endl;
		}
		userData.requestEnded = true;
	};

	bool requestEnded = false;
	wgpuInstanceRequestAdapter(
		instance,
		options,
		onAdapterRequestEnded,
		(void*)&userData
	);

	assert(userData.requestEnded);

#ifdef __EMSCRIPTEN__
	while (!userData.requestEnded) {
		emscripten_sleep(100);
	}
#endif // __EMSCRIPTEN__

	return userData.adapter;
}

WGPUDevice requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const* descriptor) {

	struct UserData {
		WGPUDevice device = nullptr;
		bool requestEnded = false;
	};
	UserData userData;

	auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, const char* message, void* pUserData) {
		UserData& userData = *reinterpret_cast<UserData*>(pUserData);
		if (status == WGPURequestDeviceStatus_Success) {
			userData.device = device;
		}else {
			std::cout << "Could not get WebGPU device: " << message << std::endl;
		}
		userData.requestEnded = true;
	};

	bool requestEnded = false;
	wgpuAdapterRequestDevice(
		adapter,
		descriptor,
		onDeviceRequestEnded,
		(void*)& userData
	);

	assert(userData.requestEnded);

#ifdef __EMSCRIPTEN__
	while (!userData.requestEnded) {
		emscripten_sleep(100);
	}
#endif // __EMSCRIPTEN__

	return userData.device;
}

void inspectDevice(WGPUDevice device) {
	std::vector<WGPUFeatureName> features;
	size_t featureCount = wgpuDeviceEnumerateFeatures(device, nullptr);
	features.resize(featureCount);
	wgpuDeviceEnumerateFeatures(device, features.data());

	std::cout << "Device features:" << std::endl;
	std::cout << std::hex;
	for (auto f : features) {
		std::cout << " - 0x" << f << std::endl;
	}
	std::cout << std::dec;

	WGPUSupportedLimits limits = {};
	limits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
	bool success = wgpuDeviceGetLimits(device, &limits) == WGPUStatus_Success;
#else
	bool success = wgpuDeviceGetLimits(device, &limits);
#endif

	if (success) {
		std::cout << "Device limits:" << std::endl;
		std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << std::endl;
		std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << std::endl;
		std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << std::endl;
		std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << std::endl;
		// [...] Extra device limits
	}
}

int main(int, char**) {
	WGPUInstanceDescriptor desc = {};
	desc.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_EMSCRIPTEN
	WGPUInstance instance = wgpuCreateInstance(nullptr);
#else //  WEBGPU_BACKEND_EMSCRIPTEN
	WGPUInstance instance = wgpuCreateInstance(&desc);
#endif //  WEBGPU_BACKEND_EMSCRIPTEN

	if (!instance) {
		std::cerr << "Could not initialize WebGPU!\n";
		return 1;
	}

	std::cout << "WGPU instance: " << instance << "\n";

	WGPURequestAdapterOptions requestAdapterOpt = {};
	requestAdapterOpt.powerPreference = WGPUPowerPreference_HighPerformance;
	requestAdapterOpt.nextInChain = nullptr;

	std::cout << "Requesting adapter..." << std::endl;
	WGPUAdapter adapter = requestAdapterSync(instance, &requestAdapterOpt);
	std::cout << "Got adapter: " << adapter << std::endl;

#ifndef __EMSCRIPTEN__
	WGPUSupportedLimits supportedLimits = {};
	supportedLimits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
	bool success = wgpuAdapterGetLimits(adapter, &supportedLimits) == WGPUStatus_Success;
#else
	bool success = wgpuAdapterGetLimits(adapter, &supportedLimits);
#endif

	if (!success) {
		std::cout << "Something went wroing during \'wgpuAdapterGetLimits()\'\n";
	}
#endif // NOT __EMSCRIPTEN__

	std::vector<WGPUFeatureName> features;
	size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);
	features.resize(featureCount);

	wgpuAdapterEnumerateFeatures(adapter, features.data());

	std::cout << "Adapter features:" << std::endl;
	std::cout << std::hex;
	for (auto f : features) {
		std::cout << " - 0x" << f << std::endl;
	}
	std::cout << std::dec;

	WGPUAdapterProperties properties = {};
	properties.nextInChain = nullptr;
	wgpuAdapterGetProperties(adapter, &properties);
	std::cout << "Adapter properties:" << std::endl;
	std::cout << " - vendorID: " << properties.vendorID << std::endl;
	if (properties.vendorName) {
		std::cout << " - vendorName: " << properties.vendorName << std::endl;
	}
	if (properties.architecture) {
		std::cout << " - architecture: " << properties.architecture << std::endl;
	}
	std::cout << " - deviceID: " << properties.deviceID << std::endl;
	if (properties.name) {
		std::cout << " - name: " << properties.name << std::endl;
	}
	if (properties.driverDescription) {
		std::cout << " - driverDescription: " << properties.driverDescription << std::endl;
	}
	std::cout << std::hex;
	std::cout << " - adapterType: 0x" << properties.adapterType << std::endl;
	std::cout << " - backendType: 0x" << properties.backendType << std::endl;
	std::cout << std::dec;

	WGPUDeviceDescriptor deviceDesc = {};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label = "DouterGPU";
	deviceDesc.requiredFeatureCount = 0;
	deviceDesc.requiredLimits = nullptr;
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = "The default queue";
	deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
		std::cout << "Device lost: reason " << reason;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	};

	std::cout << "Requesting device..." << std::endl;
	WGPUDevice device = requestDeviceSync(adapter, &deviceDesc);
	std::cout << "Got device: " << device << std::endl;

	auto onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
		};
	wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);

	inspectDevice(device);

	wgpuAdapterRelease(adapter);


	WGPUQueue queue = wgpuDeviceGetQueue(device);
	auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void* /* pUserData */) {
		std::cout << "Queued work finished with status: " << status << std::endl;
	};
	wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, nullptr /* pUserData */);

	WGPUCommandEncoderDescriptor encoderDesc = {};
	encoderDesc.nextInChain = nullptr;
	encoderDesc.label = "My command encoder";
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

	wgpuCommandEncoderInsertDebugMarker(encoder, "Do one thing");
	wgpuCommandEncoderInsertDebugMarker(encoder, "Do another thing");

	WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.nextInChain = nullptr;
	cmdBufferDescriptor.label = "Command buffer";
	WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
	wgpuCommandEncoderRelease(encoder); // release encoder after it's finished

	// Finally submit the command queue
	std::cout << "Submitting command..." << std::endl;
	wgpuQueueSubmit(queue, 1, &command);
	wgpuCommandBufferRelease(command);
	std::cout << "Command submitted." << std::endl;

	for (int i = 0; i < 5; ++i) {
		std::cout << "Tick/Poll device..." << std::endl;
#if defined(WEBGPU_BACKEND_DAWN)
		wgpuDeviceTick(device);
#elif defined(WEBGPU_BACKEND_WGPU)
		wgpuDevicePoll(device, false, nullptr);
#elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
		emscripten_sleep(100);
#endif
	}

	if (!glfwInit()) {
		std::cerr << "Could not initialize GLFW!" << std::endl;
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // <-- extra info for glfwCreateWindow
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Learn WebGPU", nullptr, nullptr);
	if (!window) {
		std::cerr << "Could not open window!" << std::endl;
		glfwTerminate();
		return 1;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	wgpuQueueRelease(queue);
	wgpuDeviceRelease(device);
	wgpuInstanceRelease(instance);	

	return 0;
}

//TODO: Follow the refactor: https://eliemichel.github.io/LearnWebGPU/getting-started/opening-a-window.html