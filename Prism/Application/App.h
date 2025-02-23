#pragma once
#include "StandardTypes.h"
#include "Graphics/Renderer.h"
#include "Graphics/Camera.h"
#include <Elos/Window/Window.h>

namespace Prism
{
	class App
	{
	public:
		void Run();

	private:
		void Init();
		void Shutdown();
		void Tick();

		void CreateMainWindow();
		void ProcessWindowEvents();
		void CreateRenderer();

	private:
		std::unique_ptr<class Elos::Window> m_window;
		std::unique_ptr<Gfx::Renderer>      m_renderer;
		std::unique_ptr<Gfx::Camera>        m_camera;
	};
}
