#include "Renderer.h"
#include "Utils/Log.h"
#include "Graphics/Utils/ResourceFactory.h"
#include <Elos/Common/Assert.h>
#include <Elos/Window/Window.h>

namespace Prism::Gfx
{
	namespace Internal
	{
		void LogAdapterInfo(const Prism::Gfx::Core::AdapterInfo& adapterInfo)
		{
			Log::Info("Adapter Name: {}"                           , adapterInfo.Description);
			Log::Info("Adapter Vendor: {}"                         , adapterInfo.DxgiDesc.VendorId);
			Log::Info("Adapter Device: {}"                         , adapterInfo.DxgiDesc.DeviceId);
			Log::Info("Adapter Subsystem: {}"                      , adapterInfo.DxgiDesc.SubSysId);
			Log::Info("Adapter Revision: {}"                       , adapterInfo.DxgiDesc.Revision);
			Log::Info("Adapter Dedicated Video Memory(GB): {:.3f}" , static_cast<f32>(adapterInfo.DedicatedVideoMemory) / 1024 / 1024 / 1024);
			Log::Info("Adapter Shared System Memory(GB): {:.3f}"   , static_cast<f32>(adapterInfo.SharedSystemMemory) / 1024 / 1024 / 1024);
			Log::Info("Adapter Dedicated System Memory(GB): {:.3f}", static_cast<f32>(adapterInfo.DedicatedSystemMemory) / 1024 / 1024 / 1024);
		}
	}

	Renderer::Renderer(Elos::Window& window, const Core::Device::DeviceDesc& deviceDesc, const Core::SwapChain::SwapChainDesc& swapChainDesc)
		: m_window(window)
	{
		CreateDevice(deviceDesc);
		CreateSwapChain(swapChainDesc);

		m_resourceFactory = std::make_unique<ResourceFactory>(m_device.get());

		Log::Info("Created Renderer");
	}

	Renderer::~Renderer()
	{
		Log::Info("Shutting down Renderer");
		m_resourceFactory.reset();
		m_swapChain.reset();
		m_device.reset();
	}

	void Renderer::ClearState() const
	{
		m_device->GetContext()->ClearState();
	}

	void Renderer::ClearBackBuffer(const f32* clearColor) const
	{
		DX11::IDeviceContext* const context = m_device->GetContext();
		DX11::IRenderTarget* const backBufferRTV = m_swapChain->GetBackBufferRTV();
		
		context->ClearRenderTargetView(backBufferRTV, clearColor);
	}

	void Renderer::SetViewports(const std::span<D3D11_VIEWPORT> viewports) const
	{
		m_device->GetContext()->RSSetViewports(static_cast<u32>(viewports.size()), viewports.data());
	}

	void Renderer::Resize(u32 width, u32 height) const
	{
		if (width == 0 || height == 0)
		{
			return;  // We cannot resize
		}

		if (m_swapChain)
		{
			// Only check for resize failure in debug builds
#if PRISM_BUILD_DEBUG
			if (auto result = m_swapChain->Resize(width, height); !result)
			{
				Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("Failed to resize DXGI Swap Chain! (Error Code: {:#x})", result.error().ErrorCode).Throw();
			}
#else
			std::ignore = m_swapChain->Resize(width, height);
#endif
		}
	}

	void Renderer::Present() const
	{
		if (m_swapChain)
		{
			// Only check for resize failure in debug builds
#if PRISM_BUILD_DEBUG
			if (auto result = m_swapChain->Present(); !result)
			{
				Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("Failed to present DXGI Swap Chain! (Error Code: {:#x})", result.error().ErrorCode).Throw();
			}
#else
			std::ignore = m_swapChain->Present();
#endif
		}
	}

	void Renderer::Flush() const
	{
		m_device->GetContext()->Flush();
	}

	void Renderer::CreateDevice(const Core::Device::DeviceDesc& deviceDesc)
	{
		if (auto result = Core::Device::Create(deviceDesc); !result)
		{
			Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("{} (Error Code: {:#x})", result.error().Message, result.error().ErrorCode).Throw();
			return;
		}
		else
		{
			m_device = std::move(result.value());
		}

		Internal::LogAdapterInfo(m_device->GetAdapterInfo());
		Log::Info("Created DX11 Device");
	}

	void Renderer::CreateSwapChain(const Core::SwapChain::SwapChainDesc& swapChainDesc)
	{
		if (auto result = Core::SwapChain::Create(*m_device, swapChainDesc); !result)
		{
			Elos::ASSERT(SUCCEEDED(result.error().ErrorCode)).Msg("{} (Error Code: {:#x})", result.error().Message, result.error().ErrorCode).Throw();
			return;
		}
		else
		{
			m_swapChain = std::move(result.value());
			Log::Info("Created DX11 Swap Chain");
		}
	}
}
