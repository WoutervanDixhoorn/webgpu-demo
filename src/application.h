#pragma once
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.hpp>

#include "graphicsDevice.h"

namespace dtr {

    class Application {
        struct WindowData {
            Application* m_Application;
        };

    private:
        bool m_IsRunning = true;

        WindowData* m_WindowData = nullptr;

        GLFWwindow* m_Window = nullptr;
        wgpu::Queue m_Queue;
        GraphicsDevice* m_Device = nullptr;
        wgpu::Surface m_WGPUSurface;

        std::vector<WGPUFeatureName> m_Features;
        wgpu::AdapterProperties m_Properties; 

    public:
        bool Initialize();

        void Terminate();

        void Update();

        inline const bool IsRunning() { return m_IsRunning; };
        
    private:
        void stopRunning() { m_IsRunning = false; }
    
        wgpu::TextureView GetNextSurfaceTextureView();
    };

}