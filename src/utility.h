#pragma once

#include <vector>

namespace dtr
{
	bool LoadGeometry(const char* filePath, std::vector<float>& vertexData, std::vector<uint16_t>& indexData);
}