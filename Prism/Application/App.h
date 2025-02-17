#pragma once
#include "StandardTypes.h"
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
	};
}