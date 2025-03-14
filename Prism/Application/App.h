#pragma once
#include "Application/AppEvents.h"
#include "Application/Scene.h"
#include "Graphics/Renderer.h"
#include <Elos/Utils/Timer.h>

namespace Prism
{
	class App
	{
	public:
		void Run();

	private:
		void Init();
		void Shutdown();
		void Tick(const Elos::Timer::TimeInfo& timeInfo);
		void Render();

		void CreateMainWindow();
		void ProcessWindowEvents();
		void CreateRenderer();
		void CreateScene();
		void InitImGui();
		void ShutdownImGui();

	private:
		AppEvents                      m_appEvents;
		bool                           m_isSolidRenderState = true;
		std::unique_ptr<Scene>	       m_scene;
		std::shared_ptr<Elos::Window>  m_window;
		std::shared_ptr<Gfx::Renderer> m_renderer;
		Elos::Timer                    m_timer;
	};
}
