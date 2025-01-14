#include "shader.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>

#include "application.h"

namespace dtr {

	Shader::Shader(const char* fileName)
		: m_FileName(fileName), m_ShaderSource(LoadShaderFromFile()) {
	}

	std::string Shader::LoadShaderFromFile()
	{
		std::ifstream shaderFile(m_FileName);
		std::stringstream shaderSource;

		if (shaderFile.is_open()) {
			while (shaderFile) {

				std::string line;
				std::getline(shaderFile, line);

				shaderSource << line << "\n";
			}
		} else {
			std::cout << "Failed loading shader file " << m_FileName << std::endl;
			assert(false);
		}

		std::cout << "Succesfully loaded " << m_FileName << std::endl;

		return shaderSource.str();
	}

	wgpu::ShaderModule Shader::GetShaderModule() const
	{
		wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
		shaderCodeDesc.chain.next = nullptr;
		shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
		shaderCodeDesc.code = GetShaderSource();

		wgpu::ShaderModuleDescriptor shaderDesc = {};
		shaderDesc.hintCount = 0;
		shaderDesc.hints = nullptr;
		shaderDesc.nextInChain = &shaderCodeDesc.chain;

		return Application::Get()->m_Context->GetNativeDevice().createShaderModule(shaderDesc);
	}

}