#include "ResourceFactory.h"
#include <Elos/Common/Assert.h>

namespace Prism::Gfx
{
	ResourceFactory::ResourceFactory(const Core::Device* device)
		: m_device(device)
	{
	}

	std::expected<std::unique_ptr<VertexBuffer>, Buffer::BufferError> ResourceFactory::CreateVertexBuffer(
		const void* vertexData, const u32 vertexCount, const u32 sizeOfVertexType, bool isDynamic) const 
	{
		std::unique_ptr<VertexBuffer> buffer(new VertexBuffer(isDynamic));

		const D3D11_BUFFER_DESC desc
		{
			.ByteWidth           = static_cast<u32>(vertexCount * sizeOfVertexType),
			.Usage               = isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
			.BindFlags           = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags      = isDynamic ? D3D11_CPU_ACCESS_WRITE : 0u,
			.MiscFlags           = 0u,
			.StructureByteStride = 0u
		};

		const D3D11_SUBRESOURCE_DATA initData
		{
			.pSysMem          = vertexData,
			.SysMemPitch      = 0,
			.SysMemSlicePitch = 0
		};

		HRESULT hr = buffer->InitInternal(m_device->GetDevice(), &desc, &initData);
		if (FAILED(hr))
		{
			return std::unexpected(Buffer::BufferError
			{
				.Type = Buffer::BufferError::Type::CreateBufferFailed,
				.ErrorCode = hr,
				.Message = "Failed to create vertex buffer"
			});
		}

		return buffer;
	}

	std::expected<std::unique_ptr<IndexBuffer>, Buffer::BufferError> ResourceFactory::CreateIndexBuffer(
		std::span<const u32> indices, bool isDynamic) const 
	{
		std::unique_ptr<IndexBuffer> buffer(new IndexBuffer(isDynamic));

		const D3D11_BUFFER_DESC desc
		{
			.ByteWidth           = static_cast<UINT>(indices.size_bytes()),
			.Usage               = isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
			.BindFlags           = D3D11_BIND_INDEX_BUFFER,
			.CPUAccessFlags      = isDynamic ? D3D11_CPU_ACCESS_WRITE : 0u,
			.MiscFlags           = 0,
			.StructureByteStride = 0u
		};

		const D3D11_SUBRESOURCE_DATA initData
		{
			.pSysMem          = indices.data(),
			.SysMemPitch      = 0,
			.SysMemSlicePitch = 0
		};

		HRESULT hr = buffer->InitInternal(m_device->GetDevice(), &desc, &initData);
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
	
	std::expected<std::unique_ptr<Mesh>, Mesh::MeshError> ResourceFactory::CreateMesh(
		const void* vertices, u32 vertexCount, std::span<const u32> indices, const Mesh::MeshDesc& desc) const 
	{
		std::unique_ptr<Mesh> mesh(new Mesh());

		auto vbResult = CreateVertexBuffer(vertices, vertexCount, desc.VertexStride, desc.DynamicVB);
		if (!vbResult)
		{
			return std::unexpected(Mesh::MeshError
			{
				.Type      = Mesh::MeshError::Type::CreateVertexBufferFailed,
				.ErrorCode = vbResult.error().ErrorCode,
				.Message   = "Failed to create vertex buffer for mesh"
			});
		}

		auto ibResult = CreateIndexBuffer(indices, desc.DynamicIB);
		if (!ibResult)
		{
			return std::unexpected(Mesh::MeshError
			{
				.Type      = Mesh::MeshError::Type::CreateIndexBufferFailed,
				.ErrorCode = ibResult.error().ErrorCode,
				.Message   = "Failed to create index buffer for mesh"
			});
		}

		mesh->m_vertexBuffer = std::move(vbResult.value());
		mesh->m_indexBuffer  = std::move(ibResult.value());

#if PRISM_BUILD_DEBUG  // Sanity checks. Should never fail
		Elos::ASSERT_NOT_NULL(mesh->GetVertexBuffer()).Throw();
		Elos::ASSERT_NOT_NULL(mesh->GetIndexBuffer()).Throw();
#endif
		mesh->m_topology                  = desc.Topology;
		mesh->m_vertexBuffer->Stride      = desc.VertexStride;
		mesh->m_vertexBuffer->VertexCount = vertexCount;
		mesh->m_indexBuffer->IndexCount   = static_cast<u32>(indices.size());

		return mesh;
	}
}