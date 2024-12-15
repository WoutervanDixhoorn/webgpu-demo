#include "shader.h"

#include <iostream>
#include <sstream>
#include <fstream>

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
				std::cout << line << std::endl;
			}
		} else {
			std::cout << "Failed loading shader file " << m_FileName << std::endl;
		}

		std::cout << "Succesfully loaded " << m_FileName << std::endl;

		return shaderSource.str();
	}

}