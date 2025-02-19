#include "Graphics/Renderer.h"
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
			Px::Log::Info("Adapter Dedicated Video Memory(GB): {:.3f}" , static_cast<Px::f32>(adapterInfo.DedicatedVideoMemory) / 1024 / 1024 / 1024);
			Px::Log::Info("Adapter Shared System Memory(GB): {:.3f}"   , static_cast<Px::f32>(adapterInfo.SharedSystemMemory) / 1024 / 1024 / 1024);
			Px::Log::Info("Adapter Dedicated System Memory(GB): {:.3f}", static_cast<Px::f32>(adapterInfo.DedicatedSystemMemory) / 1024 / 1024 / 1024);
		}
	}

	Renderer::Renderer(Elos::Window& window)
		: m_window(window)
	{
		CreateDevice(window);
		CreateSwapChain(window);
	}

	void Renderer::CreateDevice(MAYBE_UNUSED Elos::Window& window)
	{
		if (auto result = Core::Device::Create(Core::Device::DeviceDesc
			{
				.EnableDebugLayer = true,
				.EnableGPUValidation = false
			}); !result)
		{
			Elos::ASSERT(false).Msg("{}", result.error().Message).Throw();
			return;
		}
		else
		{
			m_device = std::move(result.value());
		}

		Internal::LogAdapterInfo(m_device->GetAdapterInfo());
	}

	void Renderer::CreateSwapChain(Elos::Window& window)
	{
		const Elos::WindowSize windowSize = window.GetSize();

		if (auto result = Core::SwapChain::Create(*m_device, Core::SwapChain::SwapChainDesc
			{
				.WindowHandle = window.GetHandle(),
				.Width        = windowSize.Width,
				.Height       = windowSize.Height,
				.BufferCount  = 1,
				.Format       = DXGI_FORMAT_R8G8B8A8_UNORM,
				.AllowTearing = true,
				.Fullscreen   = false
			}); !result)
		{
			Elos::ASSERT(false).Msg("{}", result.error().Message).Throw();
			return;
		}
		else
		{
			m_swapChain = std::move(result.value());
		}
	}
}