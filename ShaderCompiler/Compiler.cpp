module ShaderCompiler;

import std;
import <wrl/client.h>;
import <d3dcompiler.h>;
using Microsoft::WRL::ComPtr;
namespace SC
{
	// Custom include handler
	class ShaderIncludeHandler : public ID3DInclude
	{
	public:
		explicit ShaderIncludeHandler(const std::string& basePath)
			: m_basePath(basePath)
		{
			// Ensure base path ends with a slash
			if (!m_basePath.empty() && m_basePath.back() != '/' && m_basePath.back() != '\\')
			{
				m_basePath += '/';
			}
		}

		HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, [[maybe_unused]] LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override
		{
			std::string fullPath;

			// For local includes, prepend the base path
			if (IncludeType == D3D_INCLUDE_LOCAL)
			{
				fullPath = m_basePath + pFileName;
			}
			else // For system includes, use the file name as-is
			{
				fullPath = pFileName;
			}

			// Open and read the file
			std::ifstream file(fullPath, std::ios::binary);
			if (!file)
			{
				return E_FAIL;
			}

			// Get file size
			file.seekg(0, std::ios::end);
			size_t fileSize = static_cast<size_t>(file.tellg());
			file.seekg(0, std::ios::beg);

			// Allocate memory for file content
			char* buffer = new char[fileSize];
			if (!buffer)
			{
				return E_OUTOFMEMORY;
			}

			// Read file content
			file.read(buffer, fileSize);
			if (!file)
			{
				delete[] buffer;
				return E_FAIL;
			}

			*ppData = buffer;
			*pBytes = static_cast<UINT>(fileSize);

			return S_OK;
		}

		HRESULT __stdcall Close(LPCVOID pData) override
		{
			// Free the memory allocated in Open
			delete[] static_cast<const char*>(pData);
			return S_OK;
		}

	private:
		std::string m_basePath;
	};

	bool Compiler::Compile(const ShaderInputInfo& info)
	{
		try
		{
			std::string shaderSource = ReadShaderSource(info.InFile);
			std::vector<std::string> defineNames;
			std::vector<std::string> defineValues;

			std::vector<D3D_SHADER_MACRO> macros;
			macros.reserve(info.Defines.size() + 1);

			for (const auto& define : info.Defines)
			{
				size_t equalPos = define.find('=');

				if (equalPos != std::string::npos)
				{
					// NAME=VALUE format
					defineNames.push_back(define.substr(0, equalPos));
					defineValues.push_back(define.substr(equalPos + 1));
				}
				else
				{
					// Just NAME format (value is "1")
					defineNames.push_back(define);
					defineValues.push_back("1");
				}

				// Add to macros, using pointers to the stored strings
				macros.push_back(
				{
					defineNames.back().c_str(),
					defineValues.back().c_str()
				});
			}

			macros.push_back({ nullptr, nullptr });

			UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef PRISM_BUILD_DEBUG
			flags |= D3DCOMPILE_DEBUG;
			flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

			std::string shadersDir = "Shaders";

			ShaderIncludeHandler includeHandler(shadersDir);
			ID3DBlob* shaderBlob = nullptr;
			ID3DBlob* errorBlob = nullptr;

			HRESULT hr = D3DCompile(
				shaderSource.c_str(),
				shaderSource.size(),
				info.InFile.c_str(),
				macros.data(),
				&includeHandler,
				info.EntryPoint.c_str(),
				info.Profile.c_str(),
				flags,
				0,
				&shaderBlob,
				&errorBlob
			);

			// Check for warnings even if compilation succeeded
			if (errorBlob && errorBlob->GetBufferSize() > 0)
			{
				const char* errorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());

				// If compilation failed, this is an error
				if (FAILED(hr))
				{
					m_lastError = errorMsg;
					std::println("Shader compilation failed: {}", m_lastError);
					return false;
				}
				// If compilation succeeded but we have messages, these are warnings
				else
				{
					m_warnings = errorMsg;
					std::println("Shader compiled with warnings: {}", m_warnings);
				}
			}

			// If compilation failed but no error message
			if (FAILED(hr) && (!errorBlob || errorBlob->GetBufferSize() == 0))
			{
				m_lastError = "Shader compilation failed with HRESULT: " + std::to_string(hr);
				std::println("{}", m_lastError);
				return false;
			}

			std::filesystem::path outputPath(info.OutFile);
			std::filesystem::create_directories(outputPath.parent_path());

			WriteCompiledShader(
				info.OutFile,
				shaderBlob->GetBufferPointer(),
				shaderBlob->GetBufferSize()
			);

			return true;
		}
		catch (const std::exception& e)
		{
			m_lastError = e.what();
			std::println("Error: {}", m_lastError);
			return false;
		}
	}

	std::string Compiler::ReadShaderSource(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary);

		if (!file.is_open())
		{ 
			throw std::runtime_error("Failed to open file: " + filename);
		}

		return std::string(
			std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>()
		);
	}

	void Compiler::WriteCompiledShader(const std::string& filename, const void* data, size_t size)
	{
		std::ofstream file(filename, std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("Failed to create output file: " + filename);
		}

		file.write(static_cast<const char*>(data), size);

		if (!file)
		{
			throw std::runtime_error("Failed to write to output file: " + filename);
		}
	}

	bool CompileShader(const ShaderInputInfo& info)
	{
		Compiler compiler;
		return compiler.Compile(info);
	}
}
