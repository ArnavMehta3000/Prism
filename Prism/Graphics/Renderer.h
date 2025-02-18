#pragma once
#include "Graphics/DX11Types.h"
#include "Graphics/Core/Device.h"
#include <Elos/Common/String.h>

namespace Px::Gfx
{
	namespace Core
	{
		class Device;
	}

	class Renderer
	{
	public:
		Renderer();
	private:
		std::unique_ptr<Core::Device> m_device;
	};		
}