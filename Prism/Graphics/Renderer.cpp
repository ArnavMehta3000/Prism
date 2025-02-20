#include "Renderer.h"
#include "Utils/Log.h"
#include <Elos/Common/Assert.h>
#include <Elos/Window/Window.h>

namespace Px::Gfx
{
	namespace Internal
	{
		void LogAdapterInfo(const Px::Gfx::Core::AdapterInfo& adapterInfo)
		{
			Px::Log::Info("Adapter Name: {}"                           , adapterInfo.Description);
			Px::Log::Info("Adapter Vendor: {}"                         , adapterInfo.DxgiDesc.VendorId);
			Px::Log::Info("Adapter Device: {}"                         , adapterInfo.DxgiDesc.DeviceId);
			Px::Log::Info("Adapter Subsystem: {}"                      , adapterInfo.DxgiDesc.SubSysId);
			Px::Log::Info("Adapter Revision: {}"                       , adapterInfo.DxgiDesc.Revision);
			Px::Log::Info("Adapter Dedicated Video Memory(GB): {:.3f}" , static_cast<f32>(adapterInfo.DedicatedVideoMemory) / 1024 / 1024 / 1024);
			Px::Log::Info("Adapter Shared System Memory(GB): {:.3f}"   , static_cast<f32>(adapterInfo.SharedSystemMemory) / 1024 / 1024 / 1024);
			Px::Log::Info("Adapter Dedicated System Memory(GB): {:.3f}", static_cast<f32>(adapterInfo.DedicatedSystemMemory) / 1024 / 1024 / 1024);
		}
	}

	Renderer::Renderer(Elos::Window& window)
		: m_window(window)
	{
		CreateDevice(window);
		CreateSwapChain(window);

		Log::Info("Created Renderer");
	}

	Renderer::~Renderer()
	{
		Log::Info("Shutting down Renderer");
		m_swapChain.reset();
		m_device.reset();
	}

	void Renderer::Resize(u32 width, u32 height)
	{
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

	void Renderer::CreateDevice(MAYBE_UNUSED Elos::Window& window)
	{
		if (auto result = Core::Device::Create(Core::Device::DeviceDesc
			{
				.EnableDebugLayer = true,
			}); !result)
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

	void Renderer::CreateSwapChain(Elos::Window& window)
	{
		const Elos::WindowSize windowSize = window.GetSize();

		if (auto result = Core::SwapChain::Create(*m_device, Core::SwapChain::SwapChainDesc
			{
				.WindowHandle = window.GetHandle(),
				.Width        = windowSize.Width,
				.Height       = windowSize.Height,
				.BufferCount  = 2,
				.Format       = DXGI_FORMAT_R8G8B8A8_UNORM,
				.AllowTearing = true,
				.Fullscreen   = false
			}); !result)
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
