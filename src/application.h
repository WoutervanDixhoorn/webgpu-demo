#pragma once
#include <array>

#include <webgpu/webgpu.hpp>

#include "window.h"
#include "graphicsDevice.h"

namespace dtr {

    class Application {

        struct MyUniformData {
            std::array<float, 4> color;
            float time;

            float _pad[3];
        };

    private:
        bool m_IsRunning = true;

        Window* m_Window;
        GraphicsDevice* m_Device = nullptr;

        std::vector<WGPUFeatureName> m_Features;
        wgpu::AdapterProperties m_Properties;

        wgpu::Surface m_WGPUSurface;
        wgpu::TextureFormat m_SurfaceFormat = wgpu::TextureFormat::Undefined;

        wgpu::Queue m_Queue;
        wgpu::PipelineLayout m_PipelineLayout;
        wgpu::RenderPipeline m_Pipeline;
        wgpu::BindGroupLayout m_BindGroupLayout;
        wgpu::BindGroup m_BindGroup;
        
        wgpu::Buffer m_VertexBuffer;
        wgpu::Buffer m_IndexBuffer;
        uint32_t m_IndexCount;
        wgpu::Buffer m_UniformBuffer;

    public:
        bool Initialize();
        void Terminate();

        void Update();

        inline const bool IsRunning() { return m_IsRunning; };
        void stopRunning() { m_IsRunning = false; }

    private:
        wgpu::TextureView GetNextSurfaceTextureView();

        void InitializeBindGroups();
        void InitializeRenderPipeline();
        void InitializeBuffers();

        bool InitializeImGui(); // called in onInit
        void TerminateImGui(); // called in onFinish
        void UpdateImGui(wgpu::RenderPassEncoder renderPass); // called in onFrame
    };

}