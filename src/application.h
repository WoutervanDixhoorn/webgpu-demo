#pragma once
#include <array>

#include <webgpu/webgpu.hpp>

#include "window.h"
#include "buffer.h"
#include "graphicsDevice.h"
#include "renderer.h"

namespace dtr {
    class Application {
        struct MyUniformData {
            std::array<float, 4> color;
            float time;

            float _pad[3];
        };

    private:
        static Application* m_Instance;
        bool m_IsRunning = false;

    public:
        Window* m_Window;
        GraphicsContext* m_Context;
        Renderer* m_Renderer;

    public:
        static Application* Get(){ 
            if (m_Instance == nullptr) {
                m_Instance = Application::CreateApplication();
            }
            return m_Instance; 
        }

        void Run();

        inline const bool IsRunning() { return m_IsRunning; };
        void StopRunning() { m_IsRunning = false; }

        virtual bool Initialize() = 0;
        virtual void Terminate() = 0;
        virtual void OnGuiDraw() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnDraw(wgpu::RenderPassEncoder renderPass) = 0;

    private:
        bool initializeApplication();
        void updateApplication();
        void drawApplication();
        void terminateApplication();

    private:
        //Add this to Renderer? RenderBegin(); RenderEnd(); maybe
        wgpu::TextureView GetNextSurfaceTextureView();

        //Add to Renderer Initialization
        void initializeBindGroups();
        void initializeRenderPipeline();
        void initializeBuffers();

        //Add into ImGui Layer
        bool initializeImGui(); // called in onInit
        void terminateImGui(); // called in onFinish
        void updateImGui(wgpu::RenderPassEncoder renderPass); // called in onFrame

    public:
        static Application* CreateApplication();
    };


   
}