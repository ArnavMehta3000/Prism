#include "ResourceFactory.h"
#include <d3d11shader.h>
#include <d3dcompiler.h>
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
	
	std::expected<void, Shader::ShaderError> ResourceFactory::CreateInputLayoutFromVS(Shader::VertexShaderData* vsData) const
	{
		// Ref: https://learn.microsoft.com/en-us/windows/win32/api/d3d11shader/nn-d3d11shader-id3d11shaderreflection

#if PRISM_BUILD_DEBUG
		Elos::ASSERT_NOT_NULL(vsData).Throw();
#endif

		if (!vsData || !vsData->Shader || vsData->ByteCode.empty())
		{
			return std::unexpected(Shader::ShaderError
			{
				.Type = Shader::ShaderError::Type::CreationFailed,
				.ErrorCode = E_FAIL,
				.Message = "Failed to create input layout from vertex shader"
			});
		}

		HRESULT hr = E_FAIL;
		ComPtr<ID3D11ShaderReflection> vsReflection;
		hr = ::D3DReflect(
			vsData->ByteCode.data(),
			vsData->ByteCode.size(),
			IID_PPV_ARGS(&vsReflection));
		
		if (FAILED(hr))
		{
			return std::unexpected(Shader::ShaderError
			{
				.Type = Shader::ShaderError::Type::CreationFailed,
				.ErrorCode = hr,
				.Message = "Failed to reflect vertex shader"
			});
		}

		D3D11_SHADER_DESC desc{};
		if (FAILED(vsReflection->GetDesc(&desc)))
		{
			return std::unexpected(Shader::ShaderError
			{
				.Type = Shader::ShaderError::Type::CreationFailed,
				.ErrorCode = hr,
				.Message = "Failed to get vertex shader description"
			});
		}

		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;
		for (u32 i = 0; i < desc.InputParameters; i++)
		{
			// Get input parameter at index
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc{};
			vsReflection->GetInputParameterDesc(i, &paramDesc);

			// Create input element descripton
			D3D11_INPUT_ELEMENT_DESC elementDesc{};
			elementDesc.SemanticName         = paramDesc.SemanticName;
			elementDesc.SemanticIndex        = paramDesc.SemanticIndex;
			elementDesc.InputSlot            = 0;
			elementDesc.AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
			elementDesc.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;

			// Determine DXGI format
			if (paramDesc.Mask == 1)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)       elementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)  elementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if (paramDesc.Mask <= 3)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)       elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)  elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (paramDesc.Mask <= 7)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)       elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)  elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (paramDesc.Mask <= 15)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)       elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)  elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			inputLayout.push_back(elementDesc);
		}

		hr = m_device->GetDevice()->CreateInputLayout(
			inputLayout.data(),
			static_cast<u32>(inputLayout.size()),
			vsData->ByteCode.data(),
			vsData->ByteCode.size(),
			&vsData->Layout);

		if (FAILED(hr))
		{
			return std::unexpected(Shader::ShaderError
			{
				.Type = Shader::ShaderError::Type::CreationFailed,
				.ErrorCode = hr,
				.Message = "Failed to create input layout from vertex shader"
			});
		}

		return{};
	}
}