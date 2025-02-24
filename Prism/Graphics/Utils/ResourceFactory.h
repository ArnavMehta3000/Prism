#pragma once
#include "Graphics/Core/Device.h"
#include "Graphics/Resources/Buffers/VertexBuffer.h"
#include "Graphics/Resources/Buffers/IndexBuffer.h"
#include "Graphics/Resources/Buffers/ConstantBuffer.h"
#include "Graphics/Mesh.h"

namespace Prism::Gfx
{
	class ResourceFactory
	{
	public:
		explicit ResourceFactory(const Core::Device* device);
		~ResourceFactory() = default;

		NODISCARD std::expected<std::unique_ptr<VertexBuffer>, Buffer::BufferError> CreateVertexBuffer(const void* vertexData, const u32 vertexCount, const u32 sizeOfVertexType, bool isDynamic = false) const;
		NODISCARD std::expected<std::unique_ptr<IndexBuffer>, Buffer::BufferError> CreateIndexBuffer(std::span<const u32> indices, bool isDynamic = false) const;
		
		template <ConstantBufferType T>
		NODISCARD std::expected<std::unique_ptr<ConstantBuffer<T>>, Buffer::BufferError> CreateConstantBuffer() const;

		template <typename VertexType>
		NODISCARD std::expected<std::unique_ptr<Mesh>, Mesh::MeshError> CreateMesh(
			std::span<const VertexType> vertices,
			std::span<const u32> indices,
			const Mesh::MeshDesc& desc = Mesh::MeshDesc{}) const;
		
		NODISCARD std::expected<std::unique_ptr<Mesh>, Mesh::MeshError> CreateMesh(
			const void* vertices,
			u32 vertexCount,
			std::span<const u32> indices,
			const Mesh::MeshDesc& desc = Mesh::MeshDesc{}) const;

	private:
		const Core::Device* m_device;
	};

	template <typename VertexType>
	std::expected<std::unique_ptr<Mesh>, Mesh::MeshError> ResourceFactory::CreateMesh(
		std::span<const VertexType> vertices,
		std::span<const u32> indices,
		const Mesh::MeshDesc& desc) const
	{
		Mesh::MeshDesc finalDesc = desc;
		finalDesc.VertexStride = sizeof(VertexType);
		return CreateMesh(static_cast<const void*>(vertices.data()), static_cast<u32>(vertices.size()), indices, finalDesc);
	}

	template <ConstantBufferType T>
	std::expected<std::unique_ptr<ConstantBuffer<T>>, Buffer::BufferError> ResourceFactory::CreateConstantBuffer() const
	{
		std::unique_ptr<ConstantBuffer<T>> buffer(new ConstantBuffer<T>());

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
}