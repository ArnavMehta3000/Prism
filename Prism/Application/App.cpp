#include "App.h"
#include "Utils/Log.h"
#include <Elos/Window/Utils/WindowExtensions.h>
#include <Elos/Common/Assert.h>

namespace Px
{
	void App::Run()
	{
		Init();

		while (m_window->IsOpen())
		{
			ProcessWindowEvents();
		}

		Shutdown();
	}

	void App::Init()
	{
		Log::Info("Initializting application");
		CreateMainWindow();


		Log::Info("Initializting renderer");
		m_renderer = std::make_unique<Gfx::Renderer>(*m_window);
	}

	void App::Shutdown()
	{
		Log::Info("Shutting down application");
	}

	void App::CreateMainWindow()
	{
		m_window = std::make_unique<Elos::Window>(
			Elos::WindowCreateInfo::Default("Prism", { 1280, 720 }));

		Elos::ASSERT(m_window && m_window->GetHandle())
			.Msg("Failed to create window")
			.Throw();

		Elos::WindowExtensions::EnableDarkMode(m_window->GetHandle(), true);

	}

	void App::ProcessWindowEvents()
	{
		const auto OnWindowClosedEvent = [this](const Elos::Event::Closed&)
		{
			m_window->Close();
		};

		const auto OnWindowResizedEvent = [this](const Elos::Event::Resized& e)
		{
			Log::Info("Window resized: {0}x{1}", e.Size.Width, e.Size.Height);
			m_renderer->Resize(e.Size.Width, e.Size.Height);
		};


		m_window->HandleEvents(
			OnWindowClosedEvent,
			OnWindowResizedEvent
		);
	}
}
