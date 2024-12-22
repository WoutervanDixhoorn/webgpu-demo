#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace dtr
{
	//			1		  2
	//Sections: [points], [indices]
	//Skip: #

	bool LoadGeometry(const char* filePath, std::vector<float>& vertexData, std::vector<uint16_t>& indexData)
	{
		std::ifstream meshFile(filePath);

		if (!meshFile.is_open()) {
			return false;
		}

		vertexData.clear();
		indexData.clear();

		int section = 0;

		float vertexValue;
		uint16_t indexValue;
		std::string line;
		while (!meshFile.eof())
		{
			getline(meshFile, line);

			if (line.starts_with("#") || line.empty())
				continue;

			if (line == "[points]") {
				section = 1;
				continue;
			}
			if (line == "[indices]") {
				section = 2;
				continue;
			}

			if (section == 1) {
				std::stringstream ss(line);

				for (int i = 0; i < 5; ++i) {
					ss >> vertexValue;
					vertexData.push_back(vertexValue);
				}
			}

			if (section == 2) {
				std::istringstream iss(line);

				for (int i = 0; i < 3; i++) {
					iss >> indexValue;
					indexData.push_back(indexValue);
				}
			}
		}

		return true;
	}

}