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

	class Compiler
	{
	public:
		Compiler() = default;
		~Compiler() = default;

		bool Compile(const ShaderInputInfo& info);
		const std::string& GetLastError() const { return m_lastError; }
		const std::string& GetWarnings() const { return m_warnings; }

	private:
		std::string ReadShaderSource(const std::string& filename);
		void WriteCompiledShader(const std::string& filename, const void* data, size_t size);

	private:
		std::string m_lastError;
		std::string m_warnings;
	};

	bool CompileShader(const ShaderInputInfo& info);
}
