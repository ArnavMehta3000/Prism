#pragma once
#include "Graphics/DX11Types.h"
#include "Graphics/Core/Device.h"
#include "Graphics/Core/SwapChain.h"
#include <Elos/Common/String.h>

namespace Elos
{
	class Window;
}

namespace Px::Gfx
{
	namespace Core
	{
		class Device;
	}

	class Renderer
	{
	public:
		Renderer(Elos::Window& window);

	private:
		void CreateDevice(Elos::Window& window);
		void CreateSwapChain(Elos::Window& window);

	private:
		Elos::Window& m_window;
		std::unique_ptr<Core::Device> m_device;
		std::unique_ptr<Core::SwapChain> m_swapChain;
	};		
}