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
        GraphicsDevice* m_Device = nullptr;
        wgpu::Surface m_WGPUSurface;
        wgpu::TextureFormat m_SurfaceFormat = wgpu::TextureFormat::Undefined;
        wgpu::Queue m_Queue;
        
        wgpu::RenderPipeline m_Pipeline;

        std::vector<WGPUFeatureName> m_Features;
        wgpu::AdapterProperties m_Properties; 

        wgpu::Buffer m_VertexBuffer;
        uint32_t m_VertexCount;
    public:
        bool Initialize();

        void Terminate();

        void Update();

        inline const bool IsRunning() { return m_IsRunning; };
        
    private:
        void stopRunning() { m_IsRunning = false; }
    
        wgpu::TextureView GetNextSurfaceTextureView();

        void InitializeRenderPipeline();
        void InitializeBuffers();
    };

}