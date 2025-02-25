#pragma once
#include "Graphics/Core/Device.h"
#include "Graphics/Resources/Buffers/VertexBuffer.h"
#include "Graphics/Resources/Buffers/IndexBuffer.h"
#include "Graphics/Resources/Buffers/ConstantBuffer.h"
#include "Graphics/Resources/Shaders/Shader.h"
#include "Graphics/Mesh.h"

namespace Prism::Gfx
{
	class ResourceFactory
	{
	public:
		explicit ResourceFactory(const Core::Device* device);
		~ResourceFactory() = default;

		NODISCARD std::expected<std::shared_ptr<VertexBuffer>, Buffer::BufferError> CreateVertexBuffer(const void* vertexData, const u32 vertexCount, const u32 sizeOfVertexType, bool isDynamic = false) const;
		NODISCARD std::expected<std::shared_ptr<IndexBuffer>, Buffer::BufferError> CreateIndexBuffer(std::span<const u32> indices, bool isDynamic = false) const;
		
		template <ConstantBufferType T>
		NODISCARD std::expected<std::shared_ptr<ConstantBuffer<T>>, Buffer::BufferError> CreateConstantBuffer() const;

		template <typename VertexType>
		NODISCARD std::expected<std::shared_ptr<Mesh>, Mesh::MeshError> CreateMesh(
			std::span<const VertexType> vertices,
			std::span<const u32> indices,
			const Mesh::MeshDesc& desc = Mesh::MeshDesc{}) const;
		
		NODISCARD std::expected<std::shared_ptr<Mesh>, Mesh::MeshError> CreateMesh(
			const void* vertices,
			u32 vertexCount,
			std::span<const u32> indices,
			const Mesh::MeshDesc& desc = Mesh::MeshDesc{}) const;

		template <Shader::Type T>
		NODISCARD std::expected<std::shared_ptr<Shader>, Shader::ShaderError> CreateShader(const fs::path& path) const;

	private:
		NODISCARD std::expected<void, Shader::ShaderError> CreateInputLayoutFromVS(Shader::VertexShaderData* vsData) const;

	private:
		const Core::Device* m_device;
	};

	template <typename VertexType>
	std::expected<std::shared_ptr<Mesh>, Mesh::MeshError> ResourceFactory::CreateMesh(
		std::span<const VertexType> vertices,
		std::span<const u32> indices,
		const Mesh::MeshDesc& desc) const
	{
		Mesh::MeshDesc finalDesc = desc;
		finalDesc.VertexStride = sizeof(VertexType);
		return CreateMesh(static_cast<const void*>(vertices.data()), static_cast<u32>(vertices.size()), indices, finalDesc);
	}

	template <ConstantBufferType T>
	std::expected<std::shared_ptr<ConstantBuffer<T>>, Buffer::BufferError> ResourceFactory::CreateConstantBuffer() const
	{
		std::shared_ptr<ConstantBuffer<T>> buffer(new ConstantBuffer<T>());

		const D3D11_BUFFER_DESC desc
		{
			.ByteWidth           = static_cast<u32>(sizeof(T)),
			.Usage               = D3D11_USAGE_DYNAMIC,
			.BindFlags           = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags           = 0u,
			.StructureByteStride = 0u
		};

		HRESULT hr = buffer->InitInternal(m_device->GetDevice(), &desc, nullptr);
		if (FAILED(hr))
		{
			return std::unexpected(Buffer::BufferError
			{
				.Type = Buffer::BufferError::Type::CreateBufferFailed,
				.ErrorCode = hr,
				.Message = "Failed to create index buffer"
			});
		}

		return buffer;
	}

	template <Shader::Type T>
	std::expected<std::shared_ptr<Shader>, Shader::ShaderError> ResourceFactory::CreateShader(const fs::path& path) const
	{
		std::shared_ptr<Shader> shader = std::make_shared<Shader>(T, path);

		auto ReturnError = [](auto errorType, HRESULT hr, const Elos::String& message) -> Shader::ShaderError
		{
			return Shader::ShaderError
			{
				.Type      = errorType,
				.ErrorCode = hr,
				.Message   = message
			};
		};

		const std::vector<byte>* bytecode = nullptr;


		if constexpr (T == Shader::Type::Vertex)
		{
			Shader::VertexShaderData& vsData = shader->As<Shader::Type::Vertex>();

			bytecode = &vsData.ByteCode;
			if (bytecode->empty())
			{
				return std::unexpected(ReturnError(Shader::ShaderError::Type::FileNotFound, E_FAIL,
					"Failed to load vertex shader bytecode"));
			}

			HRESULT hr = m_device->GetDevice()->CreateVertexShader(
				bytecode->data(),
				bytecode->size(),
				nullptr,
				vsData.Shader.GetAddressOf());

			if (FAILED(hr))
			{
				return std::unexpected(ReturnError(Shader::ShaderError::Type::CreationFailed, E_FAIL,
					"Failed to create vertex shader"));
			}

			// Create input layout from compiled vertex shader
			if (auto createILResult = CreateInputLayoutFromVS(&vsData); !createILResult)
			{
				return std::unexpected(createILResult.error());
			}
		}
		else if constexpr (T == Shader::Type::Pixel)
		{
			bytecode = &shader->As<Shader::Type::Pixel>().ByteCode;
			if (bytecode->empty())
			{
				return std::unexpected(ReturnError(Shader::ShaderError::Type::FileNotFound, E_FAIL,
					"Failed to load pixel shader bytecode"));
			}

			HRESULT hr = m_device->GetDevice()->CreatePixelShader(
				bytecode->data(),
				bytecode->size(),
				nullptr,
				shader->As<Shader::Type::Pixel>().Shader.GetAddressOf());

			if (FAILED(hr))
			{
				return std::unexpected(ReturnError(Shader::ShaderError::Type::CreationFailed, E_FAIL,
					"Failed to create pixel shader"));
			}
		}
		else if constexpr (T == Shader::Type::Geometry)
		{
			bytecode = &shader->As<Shader::Type::Geometry>().ByteCode;
			if (bytecode->empty())
			{
				return std::unexpected(ReturnError(Shader::ShaderError::Type::FileNotFound, E_FAIL,
					"Failed to load geometry shader bytecode"));
			}

			HRESULT hr = m_device->GetDevice()->CreateGeometryShader(
				bytecode->data(),
				bytecode->size(),
				nullptr,
				shader->As<Shader::Type::Geometry>().Shader.GetAddressOf());

			if (FAILED(hr))
			{
				return std::unexpected(ReturnError(Shader::ShaderError::Type::CreationFailed, E_FAIL,
					"Failed to create geometry shader"));
			}
		}
		else if constexpr (T == Shader::Type::Hull)
		{
			bytecode = &shader->As<Shader::Type::Hull>().ByteCode;
			if (bytecode->empty())
			{
				return std::unexpected(ReturnError(Shader::ShaderError::Type::FileNotFound, E_FAIL,
					"Failed to load hull shader bytecode"));
			}

			HRESULT hr = m_device->GetDevice()->CreateHullShader(
				bytecode->data(),
				bytecode->size(),
				nullptr,
				shader->As<Shader::Type::Hull>().Shader.GetAddressOf());

			if (FAILED(hr))
			{
				return std::unexpected(ReturnError(Shader::ShaderError::Type::CreationFailed, E_FAIL,
					"Failed to create hull shader"));
			}
		}
		else if constexpr (T == Shader::Type::Domain)
		{
			bytecode = &shader->As<Shader::Type::Domain>().ByteCode;
			if (bytecode->empty())
			{
				return std::unexpected(ReturnError(Shader::ShaderError::Type::FileNotFound, E_FAIL,
					"Failed to load domain shader bytecode"));
			}

			HRESULT hr = m_device->GetDevice()->CreateDomainShader(
				bytecode->data(),
				bytecode->size(),
				nullptr,
				shader->As<Shader::Type::Domain>().Shader.GetAddressOf());

			if (FAILED(hr))
			{
				return std::unexpected(ReturnError(Shader::ShaderError::Type::CreationFailed, E_FAIL,
					"Failed to create domain shader"));
			}
		}
		else
		{
			std::unreachable();
		}


		return shader;
	}
}