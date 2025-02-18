#include "Graphics/Core/Device.h"
#include <Elos/Common/Assert.h>
#include <array>

namespace Px::Gfx::Core
{
	std::expected<std::unique_ptr<Device>, Device::DeviceError> Device::Create(const DeviceDesc& desc) noexcept
	{
		std::unique_ptr<Device> device(new Device());

		if (auto result = device->InitializeFactory(desc.EnableDebugLayer); !result)
		{
			return std::unexpected(result.error());
		}

		if (auto result = device->InitializeAdapter(desc.PreferredAdapter); !result)
		{
			return std::unexpected(result.error());
		}

		if (auto result = device->InitializeDevice(desc, desc.EnableDebugLayer, desc.EnableGPUValidation); !result)
		{
			return std::unexpected(result.error());
		}

		return device;
	}

	bool Device::SupportsFeatureLevel(D3D_FEATURE_LEVEL level) const noexcept
	{
		return std::find(m_supportedFeatureLevels.begin(), m_supportedFeatureLevels.end(), level) != m_supportedFeatureLevels.end();
	}
	
	std::expected<ComPtr<DX11::IBuffer>, HRESULT> Device::CreateBuffer(const D3D11_BUFFER_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData) const noexcept
	{
		return std::unexpected(E_FAIL);
	}
	
	std::expected<ComPtr<DX11::ITexture2D>, HRESULT> Device::CreateTexture2D(const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData) const noexcept
	{
		return std::unexpected(E_FAIL);
	}
	
	std::expected<ComPtr<DX11::IRenderTarget>, HRESULT> Device::CreateRenderTargetView(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC* desc) const noexcept
	{
		return std::unexpected(E_FAIL);
	}

	std::expected<ComPtr<DX11::IDepthStencil>, HRESULT> Device::CreateDepthStencilView(ID3D11Resource* resource, const D3D11_DEPTH_STENCIL_VIEW_DESC* desc) const noexcept
	{
		return std::unexpected(E_FAIL);
	}

	std::expected<ComPtr<DX11::IShaderResource>, HRESULT> Device::CreateShaderResourceView(ID3D11Resource* resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* desc) const noexcept
	{
		return std::unexpected(E_FAIL);
	}

	std::expected<ComPtr<DX11::IUnorderedAccess>, HRESULT> Device::CreateUnorderedAccessView(ID3D11Resource* resource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* desc) const noexcept
	{
		return std::unexpected(E_FAIL);
	}

	std::expected<ComPtr<DX11::ISamplerState>, HRESULT> Device::CreateSamplerState(const D3D11_SAMPLER_DESC& desc) const noexcept
	{
		return std::unexpected(E_FAIL);
	}

	std::expected<ComPtr<DX11::IRasterizerState>, HRESULT> Device::CreateRasterizerState(const D3D11_RASTERIZER_DESC& desc) const noexcept
	{
		return std::unexpected(E_FAIL);
	}

	std::expected<ComPtr<DX11::IDepthStencilState>, HRESULT> Device::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc) const noexcept
	{
		return std::unexpected(E_FAIL);
	}

	std::expected<ComPtr<DX11::IBlendState>, HRESULT> Device::CreateBlendState(const D3D11_BLEND_DESC& desc) const noexcept
	{
		return std::unexpected(E_FAIL);
	}
	
	std::expected<void, Device::DeviceError> Device::InitializeFactory(bool enableDebugLayer) noexcept
	{
		u32 factoryFlags = 0;		
		if (enableDebugLayer)
		{
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}

		HRESULT hr = ::CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_dxgiFactory));
		if (FAILED(hr))
		{
			return std::unexpected(
				Device::DeviceError
				{
					.Type      = Device::DeviceError::Type::CreateFactoryFailed,
					.ErrorCode = hr,
					.Message   = "Failed to create DXGI factory"
				});
		}

		return {};
	}
	
	std::expected<void, Device::DeviceError> Device::InitializeAdapter(uint32_t preferredAdapter) noexcept
	{
		Elos::ASSERT(m_dxgiFactory).Msg("DXGI factory is not initialized").Throw();

		u32 adapterIndex = 0;
		std::vector<ComPtr<DX11::IAdapter>> adapters;
		ComPtr<IDXGIAdapter1> currentAdapter;

		while (m_dxgiFactory->EnumAdapters1(adapterIndex, &currentAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc;
			currentAdapter->GetDesc1(&desc);

			// Skip software adapter
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				adapterIndex++;
				continue;
			}

			ComPtr<DX11::IAdapter> adapter;
			HRESULT hr = currentAdapter.As(&adapter);
			if (SUCCEEDED(hr))
			{
				adapters.push_back(std::move(adapter));
				adapterIndex++;
			}
		}

		if (adapters.empty())
		{
			return std::unexpected(
				Device::DeviceError
				{
					.Type      = Device::DeviceError::Type::EnumAdapterFailed,
					.ErrorCode = E_FAIL,
					.Message   = "No adapters found"
				});
		}

		m_dxgiAdapter.Attach(preferredAdapter < adapters.size() ? adapters[preferredAdapter].Detach() : adapters[0].Detach());
		
		DXGI_ADAPTER_DESC3 desc;
		m_dxgiAdapter->GetDesc3(&desc);
		m_adapterInfo.Description           = Elos::WStringToString(desc.Description);
		m_adapterInfo.DedicatedVideoMemory  = desc.DedicatedVideoMemory;
		m_adapterInfo.DedicatedSystemMemory = desc.DedicatedSystemMemory;
		m_adapterInfo.SharedSystemMemory    = desc.SharedSystemMemory;
		m_adapterInfo.DxgiDesc              = desc;

		return {};
	}
	
	std::expected<void, Device::DeviceError> Device::InitializeDevice(const DeviceDesc& desc, bool enableDebugLayer, bool enableGPUValidation) noexcept
	{
		constexpr std::array featureLevels{ D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

		u32 deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		deviceFlags |= enableDebugLayer 
			? (enableDebugLayer && enableGPUValidation) 
			? D3D11_CREATE_DEVICE_DEBUGGABLE : D3D11_CREATE_DEVICE_DEBUG : 0;

		ComPtr<ID3D11Device> baseDevice;
		ComPtr<ID3D11DeviceContext> baseContext;
		D3D_FEATURE_LEVEL achievedFeatureLevel;

		HRESULT hr = D3D11CreateDevice(
			m_dxgiAdapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			deviceFlags,
			featureLevels.data(),
			static_cast<u32>(featureLevels.size()),
			D3D11_SDK_VERSION,
			&baseDevice,
			&achievedFeatureLevel,
			&baseContext
		);

		if (FAILED(hr))
		{
			return std::unexpected(
				Device::DeviceError
				{
					.Type = Device::DeviceError::Type::CreateDeviceFailed,
					.ErrorCode = hr,
					.Message = "Failed to create device"
				});
		}

		m_featureLevel = achievedFeatureLevel;
		m_supportedFeatureLevels.assign(featureLevels.begin(),
			std::find(featureLevels.begin(), featureLevels.end(), achievedFeatureLevel) + 1);

		hr = baseDevice.As(&m_d3dDevice);
		if (FAILED(hr))
		{
			return std::unexpected(DeviceError
				{
					.Type      = DeviceError::Type::CreateDeviceFailed,
				 	.ErrorCode = hr,
					.Message   = "Failed to get ID3D11Device4 interface"
				});
		}

		hr = baseContext.As(&m_d3dContext);
		if (FAILED(hr))
		{
			return std::unexpected(DeviceError
				{
					.Type      = DeviceError::Type::CreateContextFailed,
					.ErrorCode = hr,
					.Message   = "Failed to get ID3D11DeviceContext4  interface"
				});
		}

		return {};
	}
}