#pragma once
#include "StandardTypes.h"
#include "Graphics/DX11Types.h"
#include <Elos/Common/String.h>
#include <Elos/Common/FunctionMacros.h>
#include <vector>
#include <expected>
#include <memory>
#include <span>

namespace Px::Gfx::Core
{
	struct AdapterInfo
	{
		Elos::String Description;
		u64 DedicatedVideoMemory;
		u64 DedicatedSystemMemory;
		u64 SharedSystemMemory;
		DXGI_ADAPTER_DESC3 DxgiDesc;
	};

	class Device
	{
	public:
		struct DeviceDesc
		{
			bool EnableDebugLayer             = false;
			bool EnableGPUValidation          = false;
			u32 PreferredAdapter              = 0;
			D3D_FEATURE_LEVEL MinFeatureLevel = D3D_FEATURE_LEVEL_11_0;
		};

		struct DeviceError
		{
			enum class Type
			{
				CreateFactoryFailed,
				EnumAdapterFailed,
				CreateDeviceFailed,
				CreateContextFailed,
				UnsupportedFeatureLevel,
				AdapterNotFound
			};

			Type Type;
			HRESULT ErrorCode;
			Elos::String Message;
		};

		NODISCARD static std::expected<std::unique_ptr<Device>, DeviceError> Create(const DeviceDesc& desc = DeviceDesc{}) noexcept;
		~Device() noexcept = default;

		NODISCARD bool SupportsFeatureLevel(D3D_FEATURE_LEVEL level) const noexcept;

		NODISCARD std::expected<ComPtr<DX11::IBuffer>           , HRESULT> CreateBuffer(const D3D11_BUFFER_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData = nullptr) const noexcept;
		NODISCARD std::expected<ComPtr<DX11::ITexture2D>        , HRESULT> CreateTexture2D(const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData = nullptr) const noexcept;
		NODISCARD std::expected<ComPtr<DX11::IRenderTarget>     , HRESULT> CreateRenderTargetView(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC* desc = nullptr) const noexcept;
		NODISCARD std::expected<ComPtr<DX11::IDepthStencil>     , HRESULT> CreateDepthStencilView(ID3D11Resource* resource, const D3D11_DEPTH_STENCIL_VIEW_DESC* desc = nullptr) const noexcept;
		NODISCARD std::expected<ComPtr<DX11::IShaderResource>   , HRESULT> CreateShaderResourceView(ID3D11Resource* resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* desc = nullptr) const noexcept;
		NODISCARD std::expected<ComPtr<DX11::IUnorderedAccess>  , HRESULT> CreateUnorderedAccessView(ID3D11Resource* resource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* desc = nullptr) const noexcept;
		NODISCARD std::expected<ComPtr<DX11::ISamplerState>     , HRESULT> CreateSamplerState(const D3D11_SAMPLER_DESC& desc) const noexcept;
		NODISCARD std::expected<ComPtr<DX11::IRasterizerState>  , HRESULT> CreateRasterizerState(const D3D11_RASTERIZER_DESC& desc) const noexcept;
		NODISCARD std::expected<ComPtr<DX11::IDepthStencilState>, HRESULT> CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc) const noexcept;
		NODISCARD std::expected<ComPtr<DX11::IBlendState>       , HRESULT> CreateBlendState(const D3D11_BLEND_DESC& desc) const noexcept;

		NODISCARD inline std::span<const D3D_FEATURE_LEVEL> GetSupportedFeatureLevels() const noexcept { return m_supportedFeatureLevels; }
		NODISCARD inline const AdapterInfo&                 GetAdapterInfo() const noexcept            { return m_adapterInfo;            }
		NODISCARD inline DX11::IDevice*                     GetDevice() const noexcept                 { return m_d3dDevice.Get();        }
		NODISCARD inline DX11::IDeviceContext*              GetContext() const noexcept                { return m_d3dContext.Get();       }
		NODISCARD inline DX11::IFactory*                    GetFactory() const noexcept                { return m_dxgiFactory.Get();      }
	
	private:
		Device() noexcept = default;

		NODISCARD std::expected<void, DeviceError> InitializeFactory(bool enableDebugLayer) noexcept;
		NODISCARD std::expected<void, DeviceError> InitializeAdapter(uint32_t preferredAdapter) noexcept;
		NODISCARD std::expected<void, DeviceError> InitializeDevice(const DeviceDesc& desc, bool enableDebugLayer, bool enableGPUValidation) noexcept;

	private:
		ComPtr<DX11::IFactory>         m_dxgiFactory;
		ComPtr<DX11::IAdapter>         m_dxgiAdapter;
		ComPtr<DX11::IDevice>          m_d3dDevice;
		ComPtr<DX11::IDeviceContext>   m_d3dContext;
		std::vector<D3D_FEATURE_LEVEL> m_supportedFeatureLevels;
		D3D_FEATURE_LEVEL              m_featureLevel{ D3D_FEATURE_LEVEL_11_0 };
		AdapterInfo                    m_adapterInfo;
	};
}