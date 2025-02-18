#pragma once
#include "StandardTypes.h"
#include "Graphics/Renderer.h"
#include <Elos/Window/Window.h>

namespace Px
{
	class App
	{
	public:
		void Run();
		
	private:
		void Init();
		void Shutdown();

		void CreateMainWindow();
		void ProcessWindowEvents();

	private:
		std::unique_ptr<Elos::Window> m_window;
		std::unique_ptr<Gfx::Renderer> m_renderer;
	};
}