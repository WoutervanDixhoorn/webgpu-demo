#pragma once
#include <webgpu/webgpu.hpp>

#include <string>

#include "buffer.h"

namespace dtr {

	class Shader {
	private:
		const char* m_FileName;
		std::string m_ShaderSource;

		UniformBuffer* m_UniformBuffer;

	public:
		Shader(const char* fileName);

		const char* GetShaderSource() const { return m_ShaderSource.c_str(); }
		wgpu::ShaderModule GetShaderModule() const;

	private:
		std::string LoadShaderFromFile();

	};

}