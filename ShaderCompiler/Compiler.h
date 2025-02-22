#pragma once
#include <string>
#include <vector>

namespace SC
{
	struct ShaderInputInfo
	{
		std::string InFile;
		std::string OutFile;
		std::string ShaderType;
		std::string Profile;
		std::string EntryPoint;
		std::vector<std::string> Defines;
	};
}
