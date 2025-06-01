#pragma once
#include "StandardTypes.h"
#include "Graphics/DX11Types.h"
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Common/String.h>
#include <utility>

namespace Prism::Gfx
{
	class Texture2D
	{
		friend class ResourceFactory;
	public:
		struct TextureError
		{
			enum class Type
			{
				CreateTextureFailed,
				CreateShaderResourceViewFailed,
				InvalidDimensions
			};

			Type Type;
			HRESULT ErrorCode;
			Elos::String Message;
		};

		struct Texture2DDesc
		{
			u32 Width          = 0;
			u32 Height         = 0;
			DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			u32 MipLevels      = 1;
			u32 ArraySize      = 1;
			u32 SampleCount    = 1;
			u32 SampleQuality  = 0;
			D3D11_USAGE Usage  = D3D11_USAGE_DEFAULT;
			u32 BindFlags      = D3D11_BIND_SHADER_RESOURCE;
			u32 CPUAccessFlags = 0;
			u32 MiscFlags      = 0;
		};

	public:
		Texture2D();
		virtual ~Texture2D();

		inline NODISCARD DX11::ITexture2D* GetTexture() const { return m_texture.Get(); }
		inline NODISCARD DX11::IShaderResource* GetSRV() const { return m_srv.Get(); }
		inline NODISCARD auto GetDimensions() const { return m_dimensions; }
		inline NODISCARD DXGI_FORMAT GetFormat() const { return m_format; }

		HRESULT CreateShaderResourceView(DX11::IDevice* device);

	private:
		HRESULT InitFromData(DX11::IDevice* device, const Texture2DDesc& desc, const D3D11_SUBRESOURCE_DATA* initData = nullptr);

	private:
		ComPtr<DX11::ITexture2D>      m_texture;
		ComPtr<DX11::IShaderResource> m_srv;
		std::pair<u32, u32>           m_dimensions;
		DXGI_FORMAT                   m_format;
	};
}