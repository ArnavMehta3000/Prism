#include "Shader.h"
#include "Graphics/Utils/DebugName.h"
#include <fstream>
#include <system_error>

namespace Prism::Gfx
{
	Shader::Shader(const Type type, const fs::path& csoPath)
	{
		auto bytecodeResult = LoadCSOFile(csoPath);
		if (!bytecodeResult)
		{
			// Initialize with empty shader variant based on type
			switch (type)
			{
			case Type::Vertex:   m_shader = VertexShaderData{};   break;
			case Type::Pixel:    m_shader = PixelShaderData{};    break;
			case Type::Compute:  m_shader = ComputeShaderData{};  break;
			case Type::Geometry: m_shader = GeometryShaderData{}; break;
			case Type::Domain:   m_shader = DomainShaderData{};   break;
			case Type::Hull:     m_shader = HullShaderData{};     break;
			}
			return;
		}

		std::vector<byte> bytecode = std::move(bytecodeResult.value());

		switch (type)
		{
		case Type::Vertex:
			m_shader = VertexShaderData{};
			std::get<VertexShaderData>(m_shader).ByteCode = std::move(bytecode);
			break;
		case Type::Pixel:
			m_shader = PixelShaderData{};
			std::get<PixelShaderData>(m_shader).ByteCode = std::move(bytecode);
			break;
		case Type::Compute:
			m_shader = ComputeShaderData{};
			std::get<ComputeShaderData>(m_shader).ByteCode = std::move(bytecode);
			break;
		case Type::Geometry:
			m_shader = GeometryShaderData{};
			std::get<GeometryShaderData>(m_shader).ByteCode = std::move(bytecode);
			break;
		case Type::Domain:
			m_shader = DomainShaderData{};
			std::get<DomainShaderData>(m_shader).ByteCode = std::move(bytecode);
			break;
		case Type::Hull:
			m_shader = HullShaderData{};
			std::get<HullShaderData>(m_shader).ByteCode = std::move(bytecode);
			break;
		}
	}

	std::expected<std::vector<byte>, Shader::ShaderError> Shader::LoadCSOFile(const fs::path& path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)
		{
			return std::unexpected(ShaderError
				{
					.Type = ShaderError::Type::FileNotFound,
					.ErrorCode = static_cast<HRESULT>(std::errc::no_such_file_or_directory),
					.Message = std::system_category().message(static_cast<i32>(std::errc::no_such_file_or_directory))
				});
		}

		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<byte> buffer(size);
		if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
		{
			return std::unexpected(ShaderError
				{
					.Type = ShaderError::Type::FileNotFound,
					.ErrorCode = static_cast<HRESULT>(std::errc::no_such_file_or_directory),
					.Message = std::system_category().message(static_cast<i32>(std::errc::no_such_file_or_directory))
				});
		}

		return buffer;
	}

	NODISCARD bool Shader::IsValid(const Shader& shader)
	{
		if (shader.Is<Shader::Type::Vertex>())
		{
			const Shader::VertexShaderData& vsData = shader.As<Shader::Type::Vertex>();
			return vsData.Shader && vsData.Layout && !vsData.ByteCode.empty();
		}
		else if (shader.Is<Shader::Type::Pixel>())
		{
			const Shader::PixelShaderData& psData = shader.As<Shader::Type::Pixel>();
			return psData.Shader && !psData.ByteCode.empty();
		}
		else if (shader.Is<Shader::Type::Compute>())
		{
			const Shader::ComputeShaderData& csData = shader.As<Shader::Type::Compute>();
			return csData.Shader && !csData.ByteCode.empty();
		}
		else if (shader.Is<Shader::Type::Geometry>())
		{
			const Shader::GeometryShaderData& gsData = shader.As<Shader::Type::Geometry>();
			return gsData.Shader && !gsData.ByteCode.empty();
		}
		else if (shader.Is<Shader::Type::Domain>())
		{
			const Shader::DomainShaderData& dsData = shader.As<Shader::Type::Domain>();
			return dsData.Shader && !dsData.ByteCode.empty();
		}

		return false;
	}

	Shader::Type Shader::GetType() const
	{
		return std::visit([](const auto& data) -> Shader::Type
		{
			return data.Type;
		}, m_shader);
	}

	void Shader::SetShaderDebugName(Elos::StringView debugName) const
	{
		std::visit([&debugName](auto& data)
		{
			if (data.Shader)
			{
				SetDebugObjectName(data.Shader.Get(), debugName);
			}
		}, m_shader);
	}
}
