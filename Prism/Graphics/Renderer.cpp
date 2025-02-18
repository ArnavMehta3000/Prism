#include "Graphics/Renderer.h"
#include "Utils/Log.h"
#include <Elos/Common/Assert.h>

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

	Renderer::Renderer()
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
}