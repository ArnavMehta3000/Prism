#include "ResourceFactory.h"
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <Elos/Common/Assert.h>
#include <directxtk/WICTextureLoader.h>

namespace Prism::Gfx
{
	ResourceFactory::ResourceFactory(const Core::Device* device)
		: m_device(device)
	{
	}

	std::expected<std::shared_ptr<VertexBuffer>, Buffer::BufferError> ResourceFactory::CreateVertexBuffer(
		const void* vertexData, const u32 vertexCount, const u32 sizeOfVertexType, bool isDynamic) const 
	{
		std::shared_ptr<VertexBuffer> buffer(new VertexBuffer(isDynamic));

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

	std::expected<std::shared_ptr<IndexBuffer>, Buffer::BufferError> ResourceFactory::CreateIndexBuffer(
		std::span<const u32> indices, bool isDynamic) const 
	{
		std::shared_ptr<IndexBuffer> buffer(new IndexBuffer(isDynamic));

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
	
	std::expected<std::shared_ptr<Mesh>, Mesh::MeshError> ResourceFactory::CreateMesh(
		const void* vertices, u32 vertexCount, std::span<const u32> indices, const Mesh::MeshDesc& desc) const 
	{
		std::shared_ptr<Mesh> mesh(new Mesh());

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
	
	std::expected<std::shared_ptr<Texture2D>, Texture2D::TextureError> ResourceFactory::CreateTexture2D(const Texture2D::Texture2DDesc& desc, const void* pixelData, const u32 rowPitch) const
	{
		std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();

		const D3D11_SUBRESOURCE_DATA initData
		{
			.pSysMem          = pixelData,
			.SysMemPitch      = rowPitch,
			.SysMemSlicePitch = 0
		};

		HRESULT hr = texture->InitFromData(m_device->GetDevice(), desc, pixelData ? &initData : nullptr);
		if (FAILED(hr))
		{
			return std::unexpected(Texture2D::TextureError
			{
				.Type      = Texture2D::TextureError::Type::CreateTextureFailed,
				.ErrorCode = hr,
				.Message   = "Failed to create texture"
			});
		}
			
		return texture;
	}

	std::expected<std::shared_ptr<Texture2D>, Texture2D::TextureError> ResourceFactory::CreateTextureFromWIC(const byte* data, u32 dataSize) const
	{
		ComPtr<ID3D11Resource> resource;
		ComPtr<ID3D11ShaderResourceView> srv;
		HRESULT hr = DirectX::CreateWICTextureFromMemoryEx(
			m_device->GetDevice(),
			m_device->GetContext(),
			reinterpret_cast<const uint8_t*>(data),
			dataSize,
			0, // Max size
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
			0, // CPU acces flags
			0, // Misc flags
			DirectX::DX11::WIC_LOADER_DEFAULT,
			&resource,
			&srv
		);

		if (FAILED(hr))
		{
			return std::unexpected(Texture2D::TextureError
			{
				.Type      = Texture2D::TextureError::Type::CreateTextureFailed,
				.ErrorCode = hr,
				.Message   = "Failed to create texture from WIC data"
			});
		}

		std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();
		if (FAILED(resource.As(&texture->m_texture)))
		{
			return std::unexpected(Texture2D::TextureError
			{
				.Type      = Texture2D::TextureError::Type::CreateTextureFailed,
				.ErrorCode = hr,
				.Message   = "Failed to cast WIC resource to ID3D11Texture2D"
			});
		}

		Elos::ASSERT(texture->m_texture.Get()).Msg("Failed to cast WIC resource to ID3D11Texture2D").Throw();

		D3D11_TEXTURE2D_DESC texDesc;
		texture->m_texture->GetDesc(&texDesc);
			
		texture->m_format     = texDesc.Format;
		texture->m_dimensions = { texDesc.Width, texDesc.Height };

		if (FAILED(srv.As(&texture->m_srv)))
		{
			return std::unexpected(Texture2D::TextureError
			{
				.Type      = Texture2D::TextureError::Type::CreateTextureFailed,
				.ErrorCode = hr,
				.Message   = "Failed to cast WIC SRV to ID3D11ShaderResourceView"
			});
		}
		

		return texture;
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