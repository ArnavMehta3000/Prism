#pragma once
#include "StandardTypes.h"
#include "Graphics/DX11Types.h"
#include <Elos/Common/String.h>
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Window/WindowHandle.h>
#include <vector>
#include <expected>
#include <memory>

namespace Prism::Gfx::Core
{
	class Device;

	class SwapChain
	{
	public:
		struct SwapChainDesc
		{
			Elos::WindowHandle WindowHandle = nullptr;
			u32 Width                       = 0;
			u32 Height                      = 0;
			u32 BufferCount                 = 2;
			u32 RefreshRateNumerator        = 0;
			u32 RefreshRateDenominator      = 0;
			u32 SyncInterval                = 1;
			DXGI_FORMAT Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
			bool AllowTearing               = true;
			bool Fullscreen                 = false;
		};

		struct SwapChainError
		{
			enum class Type
			{
				CreateSwapChainFailed,
				GetBufferFailed,
				CreateRTVFailed,
				ResizeFailed,
				PresentFailed,
				InvalidParameters
			};

			Type Type;
			HRESULT ErrorCode;
			std::string Message;
		};

		NODISCARD static std::expected<std::unique_ptr<SwapChain>, SwapChainError> Create(Device& device, const SwapChainDesc& desc) noexcept;
		~SwapChain() noexcept;

		std::expected<void, SwapChainError> Present() noexcept;
		std::expected<void, SwapChainError> Resize(u32 width, u32 height) noexcept;

		NODISCARD DX11::IRenderTarget* GetBackBufferRTV() const noexcept { return m_backBufferRTV.Get(); };
		NODISCARD inline const SwapChainDesc& GetDesc() const noexcept { return m_desc; }
		NODISCARD inline bool IsTearingSupported() const noexcept { return m_hasTearingSupport; }
	private:
		SwapChain(Device& device) noexcept;

		NODISCARD std::expected<void, SwapChainError> Initialize(const SwapChainDesc& desc) noexcept;
		NODISCARD std::expected<void, SwapChainError> CreateRenderTargetViews() noexcept;
		NODISCARD bool CheckTearingSupport() noexcept;

	private:
		Device&                     m_device;
		ComPtr<DX11::ISwapChain>    m_swapChain;
		ComPtr<DX11::IRenderTarget> m_backBufferRTV;
		SwapChainDesc               m_desc;
		bool                        m_hasTearingSupport = false;
	};
}
