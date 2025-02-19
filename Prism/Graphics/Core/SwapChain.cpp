#include "Graphics/Core/SwapChain.h"
#include "Graphics/Core/Device.h"

namespace Px::Gfx::Core
{
	std::expected<std::unique_ptr<SwapChain>, SwapChain::SwapChainError> SwapChain::Create(Device& device, const SwapChainDesc& desc) noexcept
	{
		auto ptr = new SwapChain(device);
		auto swapChain = std::unique_ptr<SwapChain>(ptr);

		if (auto result = swapChain->Initialize(desc); !result)
		{
			return std::unexpected(result.error());
		}

		return swapChain;
	}
	
	SwapChain::SwapChain(Device& device) noexcept : m_device(device) {}

	std::expected<void, SwapChain::SwapChainError> SwapChain::Present() noexcept
	{
		u32 syncInterval = m_desc.SyncInterval;
		u32 presentFlags = 0;

		if (m_hasTearingSupport && m_desc.AllowTearing && syncInterval == 0)
		{
			presentFlags |= DXGI_PRESENT_ALLOW_TEARING;
		}

		HRESULT hr = m_swapChain->Present(syncInterval, presentFlags);
		if (FAILED(hr)) 
		{
			return std::unexpected(SwapChainError
			{
				.Type      = SwapChainError::Type::PresentFailed,
				.ErrorCode = hr,
				.Message   = "Present failed"
			});
		}

		m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
		return {};
	}
	
	std::expected<void, SwapChain::SwapChainError> SwapChain::Resize(u32 width, u32 height) noexcept
	{
		if (width == 0 || height == 0) 
		{
			return std::unexpected(SwapChainError
			{
				.Type      = SwapChainError::Type::InvalidParameters,
				.ErrorCode = E_INVALIDARG,
				.Message   = "Invalid resize dimensions"
			});
		}

		// Release existing RTVs
		for (auto& rtv : m_renderTargets)
		{
			rtv.Reset();
		}

		HRESULT hr = m_swapChain->ResizeBuffers(
			m_desc.BufferCount,
			width,
			height,
			m_desc.Format,
			m_hasTearingSupport && m_desc.AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0
		);

		if (FAILED(hr)) {
			return std::unexpected(SwapChainError
			{
				.Type      = SwapChainError::Type::ResizeFailed,
				.ErrorCode = hr,
				.Message   = "ResizeBuffers failed"
			});
		}

		m_desc.Width = width;
		m_desc.Height = height;

		return CreateRenderTargetViews();
	}
	
	DX11::IRenderTarget* SwapChain::GetCurrentRTV() const noexcept
	{
		return m_renderTargets[m_currentBackBufferIndex].Get();

	}
	
	std::expected<void, SwapChain::SwapChainError> SwapChain::Initialize(const SwapChainDesc& desc) noexcept
	{
		if (!desc.WindowHandle || desc.Width == 0 || desc.Height == 0)
		{
			return std::unexpected(SwapChainError
			{
				.Type      = SwapChainError::Type::InvalidParameters,
				.ErrorCode = E_INVALIDARG,
				.Message   = "Invalid swap chain parameters"
			});
		}

		m_desc = desc;
		m_hasTearingSupport = CheckTearingSupport();

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width                 = desc.Width;
		swapChainDesc.Height                = desc.Height;
		swapChainDesc.Format                = desc.Format;
		swapChainDesc.SampleDesc.Count      = 1;
		swapChainDesc.SampleDesc.Quality    = 0;
		swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount           = desc.BufferCount;
		swapChainDesc.Scaling               = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags                 = m_hasTearingSupport && desc.AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc = {};
		fsDesc.Windowed                        = !desc.Fullscreen;
		fsDesc.RefreshRate.Numerator           = desc.RefreshRateNumerator;
		fsDesc.RefreshRate.Denominator         = desc.RefreshRateDenominator;
		fsDesc.Scaling                         = DXGI_MODE_SCALING_UNSPECIFIED;
		fsDesc.ScanlineOrdering                = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		ComPtr<IDXGISwapChain1> swapChain1;
		HRESULT hr = m_device.GetFactory()->CreateSwapChainForHwnd(
			m_device.GetDevice(),
			desc.WindowHandle,
			&swapChainDesc,
			&fsDesc,
			nullptr,
			&swapChain1
		);

		if (FAILED(hr)) 
		{
			return std::unexpected(SwapChainError
			{
				.Type      = SwapChainError::Type::CreateSwapChainFailed,
				.ErrorCode = hr,
				.Message   = "Failed to create swap chain"
			});
		}

		hr = swapChain1.As(&m_swapChain);
		if (FAILED(hr)) 
		{
			return std::unexpected(SwapChainError
			{
				.Type      = SwapChainError::Type::CreateSwapChainFailed,
				.ErrorCode = hr,
				.Message   = "Failed to get IDXGISwapChain4 interface"
			});
		}

		if (auto result = CreateRenderTargetViews(); !result) 
		{
			return std::unexpected(result.error());
		}

		return {};
	}
	
	std::expected<void, SwapChain::SwapChainError> SwapChain::CreateRenderTargetViews() noexcept
	{
		std::vector<ComPtr<ID3D11RenderTargetView>> renderTargetViews(m_desc.BufferCount);

		for (uint32_t i = 0; i < m_desc.BufferCount; ++i)
		{
			ComPtr<ID3D11Texture2D> backBuffer;
			HRESULT hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));

			if (FAILED(hr)) 
			{
				return std::unexpected(SwapChainError
				{
					.Type      = SwapChainError::Type::GetBufferFailed,
					.ErrorCode = hr,
					.Message   = "Failed to get swap chain buffer"
				});
			}

			hr = m_device.GetDevice()->CreateRenderTargetView(
				backBuffer.Get(),
				nullptr,
				&renderTargetViews[i]
			);

			if (FAILED(hr)) 
			{
				return std::unexpected(SwapChainError
				{
					.Type      = SwapChainError::Type::CreateRTVFailed,
					.ErrorCode = hr,
					.Message   = "Failed to create render target view"
				});
			}
		}

		// Upgrade all render targets
		m_renderTargets.resize(m_desc.BufferCount);
		for (u32 i = 0; i < m_desc.BufferCount; ++i)
		{
			HRESULT hr = renderTargetViews[i].As(&m_renderTargets[i]);
			if (FAILED(hr))
			{
				return std::unexpected(SwapChainError
				{
					.Type = SwapChainError::Type::CreateRTVFailed,
					.ErrorCode = hr,
					.Message = "Failed to upgrade render target view"
				});
			}
		}

		return {};
	}
	
	bool SwapChain::CheckTearingSupport() noexcept
	{
		BOOL allowTearing = FALSE;
		HRESULT hr = m_device.GetFactory()->CheckFeatureSupport(
			DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&allowTearing,
			sizeof(allowTearing)
		);
		return SUCCEEDED(hr) && allowTearing;
	}
}