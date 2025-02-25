#pragma once
#include "StandardTypes.h"
#include "Graphics/DX11Types.h"
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Common/String.h>
#include <expected>
#include <variant>
#include <vector>
#include <filesystem>

namespace Prism::Gfx
{
	namespace fs = std::filesystem;

	class Shader
	{
		friend class ResourceFactory;
	public:
		struct ShaderError
		{
			enum class Type
			{
				FileNotFound,
				InvalidShaderType,
				CreationFailed,
				BindingFailed,
				ResourceCreationFailed
			};

			Type Type;
			HRESULT ErrorCode;
			Elos::String Message;
		};

		enum class Type
		{
			Vertex,
			Hull,
			Domain,
			Geometry,
			Pixel,
			Compute
		};

#pragma region Shader Datas
		template <typename T>
		struct ShaderDataBase
		{
			ComPtr<T> Shader;
			std::vector<byte> ByteCode;

			ShaderDataBase() : Shader(nullptr) {}
			virtual ~ShaderDataBase() { Shader.Reset(); }
		};

		struct VertexShaderData : public ShaderDataBase<DX11::IVertexShader>
		{
			ComPtr<DX11::IInputLayout>  Layout;
			static constexpr Type Type = Type::Vertex;

			VertexShaderData() : ShaderDataBase() {}
			~VertexShaderData() { Layout.Reset();}
		};

		struct PixelShaderData : public ShaderDataBase<DX11::IPixelShader>
		{
			static constexpr Type Type = Type::Pixel;
		};

		struct ComputeShaderData : public ShaderDataBase<DX11::IComputeShader>
		{
			static constexpr Type Type = Type::Compute;
		};

		struct GeometryShaderData : public ShaderDataBase<DX11::IGeometryShader>
		{
			static constexpr Type Type = Type::Geometry;
		};

		struct DomainShaderData : public ShaderDataBase<DX11::IDomainShader>
		{
			static constexpr Type Type = Type::Domain;
		};

		struct HullShaderData : public ShaderDataBase<DX11::IHullShader>
		{
			static constexpr Type Type = Type::Hull;
		};
#pragma endregion

		using ShaderVariant = std::variant<
			VertexShaderData,
			PixelShaderData,
			ComputeShaderData,
			GeometryShaderData,
			DomainShaderData,
			HullShaderData>;
	public:
		Shader(const Type type, const fs::path& csoPath);

		static NODISCARD std::expected<std::vector<byte>, ShaderError> LoadCSOFile(const fs::path& path);
		static NODISCARD bool IsValid(const Shader& shader);

		Type GetType() const;

		template <Shader::Type T>
		constexpr bool Is() const;

		template <Shader::Type T>
		const auto& As() const;

		template <Shader::Type T>
		auto& As();

		void SetShaderDebugName(Elos::StringView debugName) const;

	private:
		ShaderVariant m_shader;
	};

	template <Shader::Type T>
	inline constexpr bool Shader::Is() const
	{
		if constexpr (T == Shader::Type::Vertex)   { return std::holds_alternative<VertexShaderData>(m_shader);   }
		if constexpr (T == Shader::Type::Pixel)    { return std::holds_alternative<PixelShaderData>(m_shader);    }
		if constexpr (T == Shader::Type::Compute)  { return std::holds_alternative<ComputeShaderData>(m_shader);  }
		if constexpr (T == Shader::Type::Geometry) { return std::holds_alternative<GeometryShaderData>(m_shader); }
		if constexpr (T == Shader::Type::Domain)   { return std::holds_alternative<DomainShaderData>(m_shader);   }
		if constexpr (T == Shader::Type::Hull)     { return std::holds_alternative<HullShaderData>(m_shader);     }
	}


	template <Shader::Type T>
	inline const auto& Shader::As() const
	{
		if constexpr (T == Shader::Type::Vertex)   { return std::get<VertexShaderData>(m_shader);   }
		if constexpr (T == Shader::Type::Pixel)    { return std::get<PixelShaderData>(m_shader);    }
		if constexpr (T == Shader::Type::Compute)  { return std::get<ComputeShaderData>(m_shader);  }
		if constexpr (T == Shader::Type::Geometry) { return std::get<GeometryShaderData>(m_shader); }
		if constexpr (T == Shader::Type::Domain)   { return std::get<DomainShaderData>(m_shader);   }
		if constexpr (T == Shader::Type::Hull)     { return std::get<HullShaderData>(m_shader);     }
	}

	template <Shader::Type T>
	inline auto& Shader::As()
	{
		if constexpr (T == Shader::Type::Vertex)   { return std::get<VertexShaderData>(m_shader);   }
		if constexpr (T == Shader::Type::Pixel)    { return std::get<PixelShaderData>(m_shader);    }
		if constexpr (T == Shader::Type::Compute)  { return std::get<ComputeShaderData>(m_shader);  }
		if constexpr (T == Shader::Type::Geometry) { return std::get<GeometryShaderData>(m_shader); }
		if constexpr (T == Shader::Type::Domain)   { return std::get<DomainShaderData>(m_shader);   }
		if constexpr (T == Shader::Type::Hull)     { return std::get<HullShaderData>(m_shader);     }
	}
}