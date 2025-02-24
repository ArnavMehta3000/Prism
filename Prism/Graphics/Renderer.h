#pragma once
#include "Graphics/DX11Types.h"
#include "Graphics/Core/Device.h"
#include "Graphics/Core/SwapChain.h"
#include "Graphics/Mesh.h"
#include <Elos/Common/String.h>

namespace Elos
{
	class Window;
}

namespace Prism::Gfx
{
	class Camera;
	class ResourceFactory;

	class Renderer
	{
	public:
		Renderer(Elos::Window& window, const Core::Device::DeviceDesc& deviceDesc, const Core::SwapChain::SwapChainDesc& swapChainDesc);
		~Renderer();

		NODISCARD const ResourceFactory& GetResourceFactory() const { return *m_resourceFactory; }

		void ClearState() const;
		void ClearBackBuffer(const f32* clearColor) const;
		void SetViewports(const std::span<D3D11_VIEWPORT> viewports) const;
		void Resize(u32 width, u32 height) const;
		void Present() const;
		void Flush() const;

	private:
		void CreateDevice(const Core::Device::DeviceDesc& deviceDesc);
		void CreateSwapChain(const Core::SwapChain::SwapChainDesc& swapChainDesc);
		
		NODISCARD inline Core::Device* GetDevice() const noexcept { return m_device.get(); }
		NODISCARD inline Core::SwapChain* GetSwapChain() const noexcept { return m_swapChain.get(); }

	private:
		Elos::Window&                    m_window;
		std::unique_ptr<ResourceFactory> m_resourceFactory;
		std::unique_ptr<Core::Device>    m_device;
		std::unique_ptr<Core::SwapChain> m_swapChain;
	};
}
