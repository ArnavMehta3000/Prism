#include "Texture2D.h"
#include <Elos/Common/Assert.h>

namespace Prism::Gfx
{
	Texture2D::Texture2D()
		: m_dimensions{ 0, 0 }
		, m_format(DXGI_FORMAT_UNKNOWN)
	{
	}
	
	Texture2D::~Texture2D()
	{
		m_srv.Reset();
		m_texture.Reset();
	}
	
	HRESULT Texture2D::InitFromData(DX11::IDevice* device, const Texture2DDesc& desc, const D3D11_SUBRESOURCE_DATA* initData)
	{
#if defined(PRISM_BUILD_DEBUG)
		Elos::ASSERT_NOT_NULL(device).Throw();
#endif
		if (desc.Width == 0 || desc.Height == 0)
		{
			return E_INVALIDARG;  // Cannot create a texture with no dimensions
		}

		D3D11_TEXTURE2D_DESC1 texDesc{};
		texDesc.Width              = desc.Width;
		texDesc.Height             = desc.Height;
		texDesc.MipLevels          = desc.MipLevels;
		texDesc.ArraySize          = desc.ArraySize;
		texDesc.Format             = desc.Format;
		texDesc.SampleDesc.Count   = desc.SampleCount;
		texDesc.SampleDesc.Quality = desc.SampleQuality;
		texDesc.Usage              = desc.Usage;
		texDesc.BindFlags          = desc.BindFlags;
		texDesc.CPUAccessFlags     = desc.CPUAccessFlags;
		texDesc.MiscFlags          = desc.MiscFlags;

		HRESULT hr = device->CreateTexture2D1(&texDesc, initData, &m_texture);
		if (FAILED(hr))
		{
			return hr;
		}

		if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
		{
			hr = CreateShaderResourceView(device);
			if (FAILED(hr))
			{
				m_texture.Reset();  // Invalidate the texture
				return hr;
			}
		}

		m_dimensions = std::make_pair(desc.Width, desc.Height);
		m_format = desc.Format;

		return S_OK;
	}
	
	HRESULT Texture2D::CreateShaderResourceView(DX11::IDevice* device)
	{
#if defined(PRISM_BUILD_DEBUG)
		Elos::ASSERT_NOT_NULL(device).Throw();
		Elos::ASSERT_NOT_NULL(m_texture.Get()).Throw();
#endif

		// Get the format from the texture description
		D3D11_TEXTURE2D_DESC1 desc{};
		m_texture->GetDesc1(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc{};
		srvDesc.Format                    = desc.Format;
		srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels       = desc.MipLevels;

		return device->CreateShaderResourceView1(m_texture.Get(), &srvDesc, &m_srv);
	}
}