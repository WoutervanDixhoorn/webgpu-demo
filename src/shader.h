#pragma once
#include <webgpu/webgpu.hpp>

#include <string>

namespace dtr {

	class Shader {
	private:
		const char* m_FileName;
		std::string m_ShaderSource;

	public:
		Shader(const char* fileName);

		const char* GetShaderSource() const { return m_ShaderSource.c_str(); }

	private:
		std::string LoadShaderFromFile();

	};

}