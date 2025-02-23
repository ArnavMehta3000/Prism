#pragma once
#include "Graphics/DX11Types.h"
#include "Graphics/Core/Device.h"
#include "Graphics/Core/SwapChain.h"
#include <Elos/Common/String.h>

namespace Elos
{
	class Window;
}

namespace Prism::Gfx
{
	class Renderer
	{
	public:
		Renderer(Elos::Window& window, const Core::Device::DeviceDesc& deviceDesc, const Core::SwapChain::SwapChainDesc& swapChainDesc);
		~Renderer();

		NODISCARD inline Core::Device* GetDevice() const noexcept { return m_device.get(); }
		NODISCARD inline Core::SwapChain* GetSwapChain() const noexcept { return m_swapChain.get(); }

		void Resize(u32 width, u32 height);
		void Present();

	private:
		void CreateDevice(const Core::Device::DeviceDesc& deviceDesc);
		void CreateSwapChain(const Core::SwapChain::SwapChainDesc& swapChainDesc);

	private:
		Elos::Window& m_window;
		std::unique_ptr<Core::Device> m_device;
		std::unique_ptr<Core::SwapChain> m_swapChain;
	};
}
