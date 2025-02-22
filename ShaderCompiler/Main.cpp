#include "Compiler.h"
#include <print>
#include <cxxopts.hpp>

int main(int argc, char *argv[])
{
	try
	{
		cxxopts::Options options("Prsim Shader Compiler", "DX11-HLSL shader compiler for the Prism renderer");

		options.add_options()
			("i, in", "Input shader file", cxxopts::value<std::string>())
			("t, type", "Type of shader (VS/PS/etc.)", cxxopts::value<std::string>())
			("p, profile", "Shader profile", cxxopts::value<std::string>())
			("e, entry", "Entry point", cxxopts::value<std::string>())
			("o, out", "Output filename", cxxopts::value<std::string>())
			("d, define", "Macros/Defines to compile with", cxxopts::value<std::vector<std::string>>())
			("h, help", "Print usage");

		auto result = options.parse(argc, argv);

		if (result.count("help"))
		{
			std::println("{}", options.help());
			std::exit(0);
		}

		SC::ShaderInputInfo input
		{
			.InFile = result["in"].as<std::string>(),
			.OutFile = result["out"].as<std::string>(),
			.ShaderType = result["type"].as<std::string>(),
			.Profile = result["profile"].as<std::string>(),
			.EntryPoint = result["entry"].as<std::string>(),
			.Defines = result["define"].as<std::vector<std::string>>()
		};

		std::println("InFile: {}", input.InFile);
		std::println("OutFile: {}", input.OutFile);
		std::println("ShaderType: {}", input.ShaderType);
		std::println("Profile: {}", input.Profile);
		std::println("EntryPoint: {}", input.EntryPoint);
		for(const auto& def : input.Defines)
		{
			std::println("Defines: {}", def);
		}
		// TODO: Only print error/warnings or 'file generated at '...'
		// No need of any additonal details
		std::println("");
	}
	catch (const std::exception& e)
	{
		std::println("{}", e.what());
	}
	return 0;
}
